#pragma once

#include <cstdint>
#include <cstring>

// Fixed-point price representation (scaled by 1e6 for microsecond precision)
// Price = price_scaled / 1,000,000
using Price = int64_t;
constexpr Price PRICE_SCALE = 1'000'000LL;

// Order ID type
using OrderId = uint32_t;

// Quantity type
using Quantity = uint32_t;

// Nanosecond timestamp (Linux clock_gettime)
using Timestamp = int64_t;

// HFT-optimized Order structure - tightly packed, cache-aligned
// Size: 32 bytes (fits in one cache line on most systems)
struct alignas(32) Order {
    OrderId id;              // 4 bytes
    Quantity quantity;       // 4 bytes
    Price price_scaled;      // 8 bytes (fixed-point)
    Timestamp timestamp_ns;  // 8 bytes (nanoseconds since epoch)
    uint8_t side;            // 1 byte (0=buy, 1=sell)
    uint8_t order_type;      // 1 byte (0=limit, 1=market, 2=ioc, 3=fok)
    uint16_t reserved;       // 2 bytes (padding for alignment)
    // Total: 28 bytes + 4 bytes padding = 32 bytes

    // Default constructor - zero-initialize
    Order() noexcept : id(0), quantity(0), price_scaled(0), 
                       timestamp_ns(0), side(0), order_type(0), reserved(0) {}

    // Constructor
    Order(OrderId id_, bool isBuyOrder_, Quantity quantity_, Price price_scaled_, 
          Timestamp timestamp_ns_ = 0, uint8_t order_type_ = 0) noexcept
        : id(id_), quantity(quantity_), price_scaled(price_scaled_),
          timestamp_ns(timestamp_ns_), side(isBuyOrder_ ? 0 : 1),
          order_type(order_type_), reserved(0) {}

    // Inline getters for hot path
    [[nodiscard]] inline bool isBuy() const noexcept { return side == 0; }
    [[nodiscard]] inline bool isSell() const noexcept { return side == 1; }
    
    // Price conversion helpers
    [[nodiscard]] inline double priceAsDouble() const noexcept {
        return static_cast<double>(price_scaled) / PRICE_SCALE;
    }
    
    static inline Price doubleToPrice(double price) noexcept {
        return static_cast<Price>(price * PRICE_SCALE + 0.5);
    }
    
    // Comparison operators for price-time priority
    [[nodiscard]] inline bool operator<(const Order& other) const noexcept {
        if (price_scaled != other.price_scaled) {
            return price_scaled < other.price_scaled;
        }
        return timestamp_ns < other.timestamp_ns; // FIFO for same price
    }
    
    [[nodiscard]] inline bool operator>(const Order& other) const noexcept {
        if (price_scaled != other.price_scaled) {
            return price_scaled > other.price_scaled;
        }
        return timestamp_ns < other.timestamp_ns; // FIFO for same price
    }
};

// Static assertions for size and alignment
static_assert(sizeof(Order) == 32, "Order must be exactly 32 bytes");
static_assert(alignof(Order) == 32, "Order must be 32-byte aligned");