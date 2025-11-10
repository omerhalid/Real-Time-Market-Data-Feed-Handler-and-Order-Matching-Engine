#pragma once

#include <atomic>
#include <cstddef>
#include <array>
#include <type_traits>
#include <cassert>

// Lock-free Single Producer Single Consumer (SPSC) Queue
// Based on Dmitry Vyukov's design
// https://www.1024cores.net/home/lock-free-algorithms/queues/bounded-mpmc-queue
//
// This is a bounded queue optimized for HFT scenarios where:
// - One thread produces (e.g., market data handler)
// - One thread consumes (e.g., order matcher)
// - No locks, no CAS loops in the common case
template<typename T, size_t Size>
class SPSCQueue {
    static_assert(Size > 1 && (Size & (Size - 1)) == 0, 
                  "Size must be a power of 2 greater than 1");
    static_assert(std::is_trivially_copyable_v<T>, 
                  "T must be trivially copyable for lock-free operations");

public:
    SPSCQueue() : write_pos_(0), read_pos_(0) {
        // Ensure cache line alignment to avoid false sharing
        static_assert(sizeof(SPSCQueue) % 64 == 0, 
                     "Queue should be cache-line aligned");
    }

    // Producer: Push an item (thread-safe, single producer only)
    [[nodiscard]] inline bool push(const T& item) noexcept {
        const size_t current_write = write_pos_.load(std::memory_order_relaxed);
        const size_t next_write = (current_write + 1) & (Size - 1);
        
        // Check if queue is full
        if (next_write == read_pos_.load(std::memory_order_acquire)) [[unlikely]] {
            return false; // Queue full
        }
        
        buffer_[current_write] = item;
        write_pos_.store(next_write, std::memory_order_release);
        return true;
    }

    // Producer: Try to push, returns false if full
    [[nodiscard]] inline bool try_push(const T& item) noexcept {
        return push(item);
    }

    // Consumer: Pop an item (thread-safe, single consumer only)
    [[nodiscard]] inline bool pop(T& item) noexcept {
        const size_t current_read = read_pos_.load(std::memory_order_relaxed);
        
        // Check if queue is empty
        if (current_read == write_pos_.load(std::memory_order_acquire)) [[unlikely]] {
            return false; // Queue empty
        }
        
        item = buffer_[current_read];
        const size_t next_read = (current_read + 1) & (Size - 1);
        read_pos_.store(next_read, std::memory_order_release);
        return true;
    }

    // Consumer: Try to pop, returns false if empty
    [[nodiscard]] inline bool try_pop(T& item) noexcept {
        return pop(item);
    }

    // Check if queue is empty (approximate, for monitoring)
    [[nodiscard]] inline bool empty() const noexcept {
        return read_pos_.load(std::memory_order_acquire) == 
               write_pos_.load(std::memory_order_acquire);
    }

    // Check if queue is full (approximate, for monitoring)
    [[nodiscard]] inline bool full() const noexcept {
        const size_t next_write = (write_pos_.load(std::memory_order_acquire) + 1) & (Size - 1);
        return next_write == read_pos_.load(std::memory_order_acquire);
    }

    // Get approximate size (for monitoring)
    [[nodiscard]] inline size_t size() const noexcept {
        const size_t write = write_pos_.load(std::memory_order_acquire);
        const size_t read = read_pos_.load(std::memory_order_acquire);
        if (write >= read) [[likely]] {
            return write - read;
        }
        return Size - read + write;
    }

    // Get capacity
    [[nodiscard]] static constexpr size_t capacity() noexcept {
        return Size - 1; // One slot reserved to distinguish full from empty
    }

private:
    // Align to cache lines to avoid false sharing
    alignas(64) std::atomic<size_t> write_pos_;  // Producer writes here
    alignas(64) std::array<T, Size> buffer_;     // Circular buffer
    alignas(64) std::atomic<size_t> read_pos_;   // Consumer reads here
};

// Specialization for pointer types (common in HFT)
template<typename T, size_t Size>
class SPSCQueue<T*, Size> {
    static_assert(Size > 1 && (Size & (Size - 1)) == 0, 
                  "Size must be a power of 2 greater than 1");

public:
    SPSCQueue() : write_pos_(0), read_pos_(0) {}

    [[nodiscard]] inline bool push(T* item) noexcept {
        const size_t current_write = write_pos_.load(std::memory_order_relaxed);
        const size_t next_write = (current_write + 1) & (Size - 1);
        
        if (next_write == read_pos_.load(std::memory_order_acquire)) [[unlikely]] {
            return false;
        }
        
        buffer_[current_write] = item;
        write_pos_.store(next_write, std::memory_order_release);
        return true;
    }

    [[nodiscard]] inline bool pop(T*& item) noexcept {
        const size_t current_read = read_pos_.load(std::memory_order_relaxed);
        
        if (current_read == write_pos_.load(std::memory_order_acquire)) [[unlikely]] {
            return false;
        }
        
        item = buffer_[current_read];
        const size_t next_read = (current_read + 1) & (Size - 1);
        read_pos_.store(next_read, std::memory_order_release);
        return true;
    }

    [[nodiscard]] inline bool empty() const noexcept {
        return read_pos_.load(std::memory_order_acquire) == 
               write_pos_.load(std::memory_order_acquire);
    }

    [[nodiscard]] inline size_t size() const noexcept {
        const size_t write = write_pos_.load(std::memory_order_acquire);
        const size_t read = read_pos_.load(std::memory_order_acquire);
        if (write >= read) {
            return write - read;
        }
        return Size - read + write;
    }

    [[nodiscard]] static constexpr size_t capacity() noexcept {
        return Size - 1;
    }

private:
    alignas(64) std::atomic<size_t> write_pos_;
    alignas(64) std::array<T*, Size> buffer_;
    alignas(64) std::atomic<size_t> read_pos_;
};

