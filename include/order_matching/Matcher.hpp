#pragma once

#include "OrderBook.hpp"
#include "Order.hpp"
#include "../utils/Timestamp.hpp"
#include "../utils/AsyncLogger.hpp"
#include <cstddef>

// Trade execution result
struct Trade {
    OrderId buy_order_id;
    OrderId sell_order_id;
    Quantity quantity;
    Price price_scaled;
    Timestamp timestamp_ns;
    
    Trade() noexcept = default;
    Trade(OrderId buy_id, OrderId sell_id, Quantity qty, Price price, Timestamp ts) noexcept
        : buy_order_id(buy_id), sell_order_id(sell_id), quantity(qty), 
          price_scaled(price), timestamp_ns(ts) {}
};

// HFT-optimized order matcher
// Matches orders using price-time priority (FIFO)
class Matcher {
public:
    // Match orders in the book (in-place matching)
    // Returns number of trades executed
    [[nodiscard]] size_t matchOrders(OrderBook& orderBook) noexcept {
        size_t trade_count = 0;
        
        while (orderBook.hasMatch()) [[likely]] {
            Order* best_bid = orderBook.getBestBid();
            Order* best_ask = orderBook.getBestAsk();
            
            if (!best_bid || !best_ask) [[unlikely]] {
                break;
            }
            
            // Check if prices match
            if (best_bid->price_scaled < best_ask->price_scaled) [[unlikely]] {
                break; // No match possible
            }
            
            // Determine match price (price-time priority: first order sets price)
            Price match_price = best_bid->timestamp_ns < best_ask->timestamp_ns 
                                ? best_bid->price_scaled 
                                : best_ask->price_scaled;
            
            // Determine match quantity
            Quantity match_qty = best_bid->quantity < best_ask->quantity 
                                ? best_bid->quantity 
                                : best_ask->quantity;
            
            // Execute trade
            executeTrade(orderBook, best_bid, best_ask, match_price, match_qty);
            trade_count++;
        }
        
        return trade_count;
    }
    
    // Get last executed trade (for logging/monitoring)
    [[nodiscard]] const Trade& getLastTrade() const noexcept {
        return last_trade_;
    }

private:
    Trade last_trade_;
    
    // Execute a trade between two orders
    void executeTrade(OrderBook& orderBook, Order* bid, Order* ask, 
                     Price price, Quantity qty) noexcept {
        // Record trade
        last_trade_ = Trade(bid->id, ask->id, qty, price, hft::getTimestampNs());
        
        // Log trade asynchronously (non-blocking)
        char log_msg[256];
        snprintf(log_msg, sizeof(log_msg), 
                 "Trade: %u @ %ld (Bid:%u Ask:%u Qty:%u)",
                 qty, price, bid->id, ask->id, qty);
        hft::getLogger().info(log_msg);
        
        // Reduce quantities
        bid->quantity -= qty;
        ask->quantity -= qty;
        
        // Remove fully filled orders (partial fills are more common)
        if (bid->quantity == 0) [[unlikely]] {
            orderBook.cancelOrder(bid->id);
        }
        if (ask->quantity == 0) [[unlikely]] {
            orderBook.cancelOrder(ask->id);
        }
        
        // Note: In a real HFT system, you'd also:
        // - Update position tracking
        // - Send trade confirmations
        // - Update P&L
    }
};