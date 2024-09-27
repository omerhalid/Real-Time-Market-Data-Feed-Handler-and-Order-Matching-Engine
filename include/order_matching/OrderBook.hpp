#pragma once

#include <iostream>
#include <queue>
#include <vector>
#include <functional>
#include <unordered_map>
#include "Order.hpp" // Change to .hpp

// Alias for Order<double>
using OrderDouble = Order<double>;

// Comparison functor for buy orders (max-heap)
struct BuyOrderCompare {
    bool operator()(const OrderDouble& lhs, const OrderDouble& rhs) const {
        return lhs.price < rhs.price; // Higher price has higher priority
    } // timestamp can be added here
};

// Comparison functor for sell orders (min-heap)
struct SellOrderCompare {
    bool operator()(const OrderDouble& lhs, const OrderDouble& rhs) const {
        return lhs.price > rhs.price; // Lower price has higher priority
    }
};

class OrderBook
{
private:
    std::priority_queue<OrderDouble, std::vector<OrderDouble>, BuyOrderCompare> buyOrders;
    std::priority_queue<OrderDouble, std::vector<OrderDouble>, SellOrderCompare> sellOrders;

    std::unordered_map<int, OrderDouble> orderMap;

    template<typename Compare>
    void removeFromHeap(std::priority_queue<OrderDouble, std::vector<OrderDouble>, Compare> &heap, int id);

public:
    void AddOrder(int id_, int quantity_, double price_, bool isBuyOrder_);
    void CancelOrder(int id);
    void modifyOrder(const int& id, const double& newPrice, const int& newQuantity);
    auto getBuyOrders() { return buyOrders; }
    auto getSellOrders() { return sellOrders; }
};