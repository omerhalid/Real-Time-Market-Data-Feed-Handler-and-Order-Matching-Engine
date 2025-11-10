#pragma once

#include "SPSCQueue.hpp"
#include "Timestamp.hpp"
#include <string>
#include <thread>
#include <atomic>
#include <fstream>
#include <iostream>
#include <sstream>
#include <memory>
#include <chrono>
#include <cstring>
#include <ctime>

#ifdef __linux__
#include <sys/syscall.h>
#include <unistd.h>
#endif

namespace hft {

// Log levels
enum class LogLevel : uint8_t {
    TRACE = 0,
    DEBUG = 1,
    INFO = 2,
    WARN = 3,
    ERROR = 4,
    FATAL = 5
};

// Log message structure (cache-aligned for performance)
struct alignas(64) LogMessage {
    LogLevel level;
    int64_t timestamp_ns;
    uint32_t thread_id;
    char message[256];  // Fixed-size buffer to avoid allocations
    
    LogMessage() noexcept : level(LogLevel::INFO), timestamp_ns(0), thread_id(0) {
        message[0] = '\0';
    }
    
    LogMessage(LogLevel lvl, const char* msg) noexcept 
        : level(lvl), timestamp_ns(hft::getTimestampNs()), thread_id(getThreadId()) {
        strncpy(message, msg, sizeof(message) - 1);
        message[sizeof(message) - 1] = '\0';
    }
};

// Async logger using SPSC queue
// Producer: Hot path threads push log messages
// Consumer: Background thread writes to file/stdout
class AsyncLogger {
public:
    static constexpr size_t QUEUE_SIZE = 65536; // 64K messages
    
    AsyncLogger() 
        : running_(false), 
          log_file_(nullptr),
          min_level_(LogLevel::INFO) {}
    
    ~AsyncLogger() {
        stop();
    }
    
    // Initialize logger (start background thread)
    bool start(const std::string& log_file_path = "") {
        if (running_.load(std::memory_order_acquire)) {
            return false; // Already running
        }
        
        if (!log_file_path.empty()) {
            log_file_ = std::make_unique<std::ofstream>(log_file_path, 
                                                        std::ios::out | std::ios::app);
            if (!log_file_->is_open()) {
                return false;
            }
        }
        
        running_.store(true, std::memory_order_release);
        log_thread_ = std::thread(&AsyncLogger::logWorker, this);
        
        return true;
    }
    
    // Stop logger (flush remaining messages)
    void stop() {
        if (!running_.load(std::memory_order_acquire)) {
            return;
        }
        
        running_.store(false, std::memory_order_release);
        
        if (log_thread_.joinable()) {
            log_thread_.join();
        }
        
        // Flush remaining messages
        flush();
        
        if (log_file_) {
            log_file_->close();
            log_file_.reset();
        }
    }
    
    // Set minimum log level
    void setLevel(LogLevel level) noexcept {
        min_level_.store(level, std::memory_order_release);
    }
    
    // Log methods (non-blocking, lock-free)
    void log(LogLevel level, const char* message) noexcept {
        if (level < min_level_.load(std::memory_order_acquire)) {
            return; // Below minimum level
        }
        
        if (!running_.load(std::memory_order_acquire)) {
            return; // Logger not started
        }
        
        LogMessage msg(level, message);
        
        // Try to push, drop if queue is full (non-blocking)
        if (!queue_.push(msg)) {
            // Queue full - could increment drop counter here
            // In production, might want to use a larger queue or handle differently
        }
    }
    
    // Convenience methods
    void trace(const char* message) noexcept { log(LogLevel::TRACE, message); }
    void debug(const char* message) noexcept { log(LogLevel::DEBUG, message); }
    void info(const char* message) noexcept { log(LogLevel::INFO, message); }
    void warn(const char* message) noexcept { log(LogLevel::WARN, message); }
    void error(const char* message) noexcept { log(LogLevel::ERROR, message); }
    void fatal(const char* message) noexcept { log(LogLevel::FATAL, message); }
    
    // Format and log (non-blocking)
    template<typename... Args>
    void logf(LogLevel level, const char* format, Args... args) noexcept {
        if (level < min_level_.load(std::memory_order_acquire)) {
            return;
        }
        
        char buffer[256];
        snprintf(buffer, sizeof(buffer), format, args...);
        log(level, buffer);
    }
    
    // Flush all pending messages (blocking, use sparingly)
    void flush() {
        LogMessage msg;
        while (queue_.pop(msg)) {
            writeLogMessage(msg);
        }
    }
    
    // Get queue size (for monitoring)
    size_t queueSize() const noexcept {
        return queue_.size();
    }

private:
    std::atomic<bool> running_;
    std::atomic<LogLevel> min_level_;
    SPSCQueue<LogMessage, QUEUE_SIZE> queue_;
    std::thread log_thread_;
    std::unique_ptr<std::ofstream> log_file_;
    
    // Background worker thread
    void logWorker() {
        LogMessage msg;
        constexpr int64_t flush_interval_ns = 100'000'000LL; // 100ms
        int64_t last_flush = hft::getMonotonicNs();
        
        while (running_.load(std::memory_order_acquire) || !queue_.empty()) {
            // Try to pop a message
            if (queue_.pop(msg)) {
                writeLogMessage(msg);
                last_flush = hft::getMonotonicNs();
            } else {
                // Queue empty, check if we should flush
                int64_t now = hft::getMonotonicNs();
                if (now - last_flush > flush_interval_ns) {
                    if (log_file_) {
                        log_file_->flush();
                    }
                    last_flush = now;
                }
                
                // Small sleep to avoid busy-waiting
                std::this_thread::sleep_for(std::chrono::microseconds(10));
            }
        }
        
        // Final flush
        flush();
        if (log_file_) {
            log_file_->flush();
        }
    }
    
    // Write log message to output
    void writeLogMessage(const LogMessage& msg) {
        // Format: [LEVEL] [TIMESTAMP] [THREAD] MESSAGE
        std::ostringstream oss;
        oss << "[" << levelToString(msg.level) << "] "
            << "[" << formatTimestamp(msg.timestamp_ns) << "] "
            << "[T" << msg.thread_id << "] "
            << msg.message << "\n";
        
        std::string log_line = oss.str();
        
        if (log_file_) {
            *log_file_ << log_line;
        } else {
            std::cout << log_line;
        }
    }
    
    // Convert log level to string
    static const char* levelToString(LogLevel level) noexcept {
        switch (level) {
            case LogLevel::TRACE: return "TRACE";
            case LogLevel::DEBUG: return "DEBUG";
            case LogLevel::INFO:  return "INFO ";
            case LogLevel::WARN:  return "WARN ";
            case LogLevel::ERROR: return "ERROR";
            case LogLevel::FATAL: return "FATAL";
            default: return "UNKNOWN";
        }
    }
    
    // Format timestamp (nanoseconds to readable format)
    static std::string formatTimestamp(int64_t ns) {
        int64_t seconds = ns / 1'000'000'000LL;
        int64_t nanos = ns % 1'000'000'000LL;
        
        std::time_t time = seconds;
        std::tm* tm = std::localtime(&time);
        
        char buffer[64];
        snprintf(buffer, sizeof(buffer), "%04d-%02d-%02d %02d:%02d:%02d.%09ld",
                 tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
                 tm->tm_hour, tm->tm_min, tm->tm_sec, nanos);
        
        return std::string(buffer);
    }
    
    // Get thread ID (Linux-specific)
    static uint32_t getThreadId() noexcept {
#ifdef __linux__
        return static_cast<uint32_t>(syscall(SYS_gettid));
#else
        return static_cast<uint32_t>(
            std::hash<std::thread::id>{}(std::this_thread::get_id()));
#endif
    }
};

// Global logger instance (thread-safe for single producer)
inline AsyncLogger& getLogger() {
    static AsyncLogger logger;
    return logger;
}

// Convenience macros (compile-time level checking)
#define LOG_TRACE(...) hft::getLogger().trace(__VA_ARGS__)
#define LOG_DEBUG(...) hft::getLogger().debug(__VA_ARGS__)
#define LOG_INFO(...)  hft::getLogger().info(__VA_ARGS__)
#define LOG_WARN(...)  hft::getLogger().warn(__VA_ARGS__)
#define LOG_ERROR(...) hft::getLogger().error(__VA_ARGS__)
#define LOG_FATAL(...) hft::getLogger().fatal(__VA_ARGS__)

#define LOG_TRACE_F(...) hft::getLogger().logf(hft::LogLevel::TRACE, __VA_ARGS__)
#define LOG_DEBUG_F(...) hft::getLogger().logf(hft::LogLevel::DEBUG, __VA_ARGS__)
#define LOG_INFO_F(...)  hft::getLogger().logf(hft::LogLevel::INFO, __VA_ARGS__)
#define LOG_WARN_F(...)  hft::getLogger().logf(hft::LogLevel::WARN, __VA_ARGS__)
#define LOG_ERROR_F(...) hft::getLogger().logf(hft::LogLevel::ERROR, __VA_ARGS__)
#define LOG_FATAL_F(...) hft::getLogger().logf(hft::LogLevel::FATAL, __VA_ARGS__)

} // namespace hft

