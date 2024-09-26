#include <iostream>
#include "Order.cpp" // Change to .hpp
#include <queue>
#include <vector>
#include <functional>

// Comparison functor for buy orders (max-heap)
struct BuyOrderCompare {
    bool operator()(const Order<double>& lhs, const Order<double>& rhs) const {
        return lhs.price < rhs.price; // Higher price has higher priority
    } // timestamp can be added here
};

// Comparison functor for sell orders (min-heap)
struct SellOrderCompare {
    bool operator()(const Order<double>& lhs, const Order<double>& rhs) const {
        return lhs.price > rhs.price; // Lower price has higher priority
    }
};

class OrderBook
{
private:
    std::priority_queue<Order<double>, std::vector<Order<double>>, BuyOrderCompare> buyOrders;
    std::priority_queue<Order<double>, std::vector<Order<double>>, SellOrderCompare> sellOrders;

    std::unordered_map<int, Order<double>> orderMap;

    void removeFromHeap()
    {
        // Implementation for removing from heap
    }

public:
    void AddOrder(int id_, int quantity_, double price_, bool isBuyOrder_)
    {
        Order<double> newOrder(id_, quantity_, price_, isBuyOrder_);
        orderMap[id_] = newOrder;
        
        if (isBuyOrder_)
        {
            buyOrders.push(newOrder);
        }
        else
        {
            sellOrders.push(newOrder);
        }
    }
};