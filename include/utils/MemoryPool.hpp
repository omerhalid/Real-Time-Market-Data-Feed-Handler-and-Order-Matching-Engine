#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <array>
#include <atomic>
#include <memory>
#include "../order_matching/Order.hpp"

#ifdef __linux__
#include "../utils/LinuxOptimizations.hpp"
#endif

// Lock-free memory pool for Order objects
// Pre-allocates a fixed-size pool to avoid dynamic allocations in hot path
template<size_t PoolSize = 1024 * 1024> // 1M orders by default
class OrderMemoryPool {
public:
    OrderMemoryPool(int numa_node = -1) {
#ifdef __linux__
        // NUMA-aware allocation: allocate pool on specific NUMA node
        if (numa_node >= 0) {
            void* numa_ptr = hft::allocateOnNumaNode(sizeof(Order) * PoolSize, numa_node);
            if (numa_ptr) {
                // Use NUMA-allocated memory (would need custom allocator)
                // For now, fall through to regular allocation
            }
        }
#endif
        // Pre-allocate all orders
        for (size_t i = 0; i < PoolSize; ++i) {
            free_list_[i].store(&pool_[i], std::memory_order_relaxed);
        }
        next_free_.store(0, std::memory_order_relaxed);
    }

    // Allocate an order from the pool (lock-free)
    [[nodiscard]] inline Order* allocate() noexcept {
        size_t idx = next_free_.fetch_add(1, std::memory_order_relaxed);
        if (idx < PoolSize) [[likely]] {
            Order* order = &pool_[idx];
            // Zero-initialize
            std::memset(order, 0, sizeof(Order));
            return order;
        }
        // Pool exhausted - fallback to heap (shouldn't happen in production)
        return new Order();
    }

    // Deallocate (no-op for pool, but provided for interface compatibility)
    inline void deallocate(Order* order) noexcept {
        // In a real HFT system, you might want to recycle orders
        // For simplicity, we just mark as available
        if (order >= pool_.data() && order < pool_.data() + PoolSize) [[likely]] {
            // Order is from pool, can be reused
            std::memset(order, 0, sizeof(Order));
        } else [[unlikely]] {
            // Order was allocated from heap
            delete order;
        }
    }

    // Reset the pool (for testing/benchmarking)
    void reset() noexcept {
        next_free_.store(0, std::memory_order_relaxed);
    }

private:
    alignas(64) std::array<Order, PoolSize> pool_;  // Cache-aligned pool
    alignas(64) std::array<std::atomic<Order*>, PoolSize> free_list_;  // Free list
    alignas(64) std::atomic<size_t> next_free_;  // Next free index
};

// Global pool instance (thread-local for multi-threaded scenarios)
thread_local inline OrderMemoryPool<> g_order_pool;

