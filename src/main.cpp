#include <iostream>
#include <cstdlib>
#include <thread>
#include <chrono>
#include "order_matching/Order.hpp"
#include "order_matching/OrderBook.hpp"
#include "order_matching/Matcher.hpp"
#include "utils/MemoryPool.hpp"
#include "utils/Timestamp.hpp"
#include "utils/LinuxOptimizations.hpp"
#include "utils/SPSCQueue.hpp"
#include "utils/AsyncLogger.hpp"

#ifdef __linux__
#include "networking/EpollServer.hpp"
#endif

// Forward declaration
void demonstrateSPSCQueue();

// Example usage of the HFT-optimized order matching engine
int main(int argc, char* argv[]) {
    // Calibrate RDTSC for accurate cycle-to-nanosecond conversion
    hft::RDTSCCalibrator::calibrate();
    
    // Initialize async logger first
    hft::getLogger().setLevel(hft::LogLevel::INFO);
    hft::getLogger().start("market_engine.log");  // Log to file
    
    LOG_INFO("========================================");
    LOG_INFO("HFT Market Data Engine - Linux Optimized");
    LOG_INFO("========================================");
    
#ifdef __linux__
    // Initialize Linux-specific optimizations
    LOG_INFO("Initializing HFT optimizations...");
    if (hft::initializeHftOptimizations(0, 50)) {
        LOG_INFO("  ✓ CPU affinity set");
        LOG_INFO("  ✓ Memory locked");
    } else {
        LOG_WARN("  ⚠ Some optimizations require root privileges");
    }
    LOG_INFO_F("  CPU cores: %d", hft::getCpuCount());
#else
    LOG_WARN("⚠ Running on non-Linux system - HFT optimizations unavailable");
#endif
    
    // Create order book and matcher
    OrderBook orderBook;
    Matcher matcher;
    
    // Example: Add some orders
    LOG_INFO("Adding sample orders...");
    
    // Create orders using memory pool
    Order* buy_order1 = g_order_pool.allocate();
    *buy_order1 = Order(1, true, 100, Order::doubleToPrice(100.50), hft::getTimestampNs());
    orderBook.addOrder(buy_order1);
    LOG_INFO_F("  Added buy order: ID=1, Qty=100, Price=$100.50");
    
    Order* buy_order2 = g_order_pool.allocate();
    *buy_order2 = Order(2, true, 50, Order::doubleToPrice(100.75), hft::getTimestampNs());
    orderBook.addOrder(buy_order2);
    LOG_INFO_F("  Added buy order: ID=2, Qty=50, Price=$100.75");
    
    Order* sell_order1 = g_order_pool.allocate();
    *sell_order1 = Order(3, false, 75, Order::doubleToPrice(100.25), hft::getTimestampNs());
    orderBook.addOrder(sell_order1);
    LOG_INFO_F("  Added sell order: ID=3, Qty=75, Price=$100.25");
    
    // Display order book state
    LOG_INFO("Order Book State:");
    LOG_INFO_F("  Best Bid: $%.2f (Qty: %u)", 
               orderBook.getBestBidPrice() / 1e6,
               orderBook.getBestBid() ? orderBook.getBestBid()->quantity : 0);
    LOG_INFO_F("  Best Ask: $%.2f (Qty: %u)",
               orderBook.getBestAskPrice() / 1e6,
               orderBook.getBestAsk() ? orderBook.getBestAsk()->quantity : 0);
    LOG_INFO_F("  Spread: $%.2f", orderBook.getSpread() / 1e6);
    
    // Match orders - using RDTSC for ultra-fast latency measurement
    LOG_INFO("Matching orders...");
    hft::LatencyTimerRDTSC timer_rdtsc;  // Ultra-fast RDTSC-based timer
    size_t trades = matcher.matchOrders(orderBook);
    int64_t latency_us = timer_rdtsc.elapsedUs();
    uint64_t latency_cycles = timer_rdtsc.elapsedCycles();
    
    LOG_INFO_F("  Executed %zu trade(s) in %ld microseconds (%llu cycles)", 
               trades, latency_us, latency_cycles);
    
    if (trades > 0) {
        const Trade& last_trade = matcher.getLastTrade();
        LOG_INFO_F("  Last trade: %u @ $%.2f",
                   last_trade.quantity, last_trade.price_scaled / 1e6);
    }
    
    // Display updated order book
    LOG_INFO("Order Book State After Matching:");
    LOG_INFO_F("  Best Bid: $%.2f (Qty: %u)",
               orderBook.getBestBidPrice() / 1e6,
               orderBook.getBestBid() ? orderBook.getBestBid()->quantity : 0);
    LOG_INFO_F("  Best Ask: $%.2f (Qty: %u)",
               orderBook.getBestAskPrice() / 1e6,
               orderBook.getBestAsk() ? orderBook.getBestAsk()->quantity : 0);
    
    // Demonstrate SPSC queue for inter-thread communication
    LOG_INFO("\nDemonstrating SPSC Queue...");
    demonstrateSPSCQueue();
    
    // Flush logger before exit
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Allow logger to flush
    hft::getLogger().flush();
    hft::getLogger().stop();
    
    return 0;
}

// Demonstrate SPSC queue usage for inter-thread communication
void demonstrateSPSCQueue() {
    using OrderQueue = SPSCQueue<Order*, 1024>;
    OrderQueue order_queue;
    
    // Producer thread (simulates market data handler)
    std::atomic<bool> producer_done(false);
    std::thread producer([&order_queue, &producer_done]() {
        for (int i = 0; i < 10; ++i) {
            Order* order = g_order_pool.allocate();
            *order = Order(100 + i, (i % 2 == 0), 10 + i, 
                          Order::doubleToPrice(100.0 + i * 0.1), 
                          hft::getTimestampNs());
            
            if (order_queue.push(order)) {
                LOG_DEBUG_F("Producer: Pushed order %d", order->id);
            } else {
                LOG_WARN("Producer: Queue full!");
            }
            
            std::this_thread::sleep_for(std::chrono::microseconds(100));
        }
        producer_done.store(true, std::memory_order_release);
    });
    
    // Consumer thread (simulates order matcher)
    std::thread consumer([&order_queue, &producer_done]() {
        int processed = 0;
        while (!producer_done.load(std::memory_order_acquire) || !order_queue.empty()) {
            Order* order = nullptr;
            if (order_queue.pop(order)) {
                LOG_DEBUG_F("Consumer: Processed order %d (Price: $%.2f)",
                           order->id, order->priceAsDouble());
                processed++;
            } else {
                std::this_thread::sleep_for(std::chrono::microseconds(10));
            }
        }
        LOG_INFO_F("Consumer: Processed %d orders", processed);
    });
    
    producer.join();
    consumer.join();
    
    LOG_INFO_F("SPSC Queue demonstration complete. Final queue size: %zu", order_queue.size());
}