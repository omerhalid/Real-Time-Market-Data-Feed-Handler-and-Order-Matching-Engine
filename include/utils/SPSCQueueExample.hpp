#pragma once

// Example usage patterns for SPSC Queue in HFT systems

/*
// Example 1: Market Data Handler -> Order Matcher
// Producer thread (market data handler)
void marketDataHandler(SPSCQueue<Order*, 65536>& order_queue) {
    while (running) {
        Order* order = parseMarketDataMessage();
        if (order) {
            order_queue.push(order);  // Non-blocking, lock-free
        }
    }
}

// Consumer thread (order matcher)
void orderMatcher(SPSCQueue<Order*, 65536>& order_queue, OrderBook& book) {
    Order* order = nullptr;
    while (running) {
        if (order_queue.pop(order)) {
            book.addOrder(order);
            matcher.matchOrders(book);
        } else {
            // Queue empty, can do other work or sleep briefly
            std::this_thread::sleep_for(std::chrono::nanoseconds(100));
        }
    }
}

// Example 2: Trade Logger (async logging)
SPSCQueue<Trade, 1024> trade_log_queue;

// Hot path: Record trade (non-blocking)
void recordTrade(const Trade& trade) {
    trade_log_queue.push(trade);  // Never blocks
}

// Background thread: Write trades to disk
void tradeLoggerThread() {
    Trade trade;
    while (running) {
        if (trade_log_queue.pop(trade)) {
            writeTradeToDisk(trade);
        }
    }
}

// Example 3: Latency measurements
SPSCQueue<LatencyMeasurement, 4096> latency_queue;

// Hot path: Record latency (non-blocking)
void recordLatency(int64_t latency_ns) {
    LatencyMeasurement m{latency_ns, getTimestampNs()};
    latency_queue.push(m);  // Never blocks hot path
}

// Background thread: Aggregate and report statistics
void latencyReporterThread() {
    LatencyMeasurement m;
    std::vector<int64_t> latencies;
    while (running) {
        while (latency_queue.pop(m)) {
            latencies.push_back(m.latency_ns);
        }
        if (latencies.size() >= 1000) {
            calculatePercentiles(latencies);
            latencies.clear();
        }
    }
}
*/

