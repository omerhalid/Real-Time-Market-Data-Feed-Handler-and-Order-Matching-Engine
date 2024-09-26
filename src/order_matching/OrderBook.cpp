#include <iostream>
#include "Order.cpp" // Change to .hpp
#include <queue>
#include <vector>
#include <functional>
#include <unordered_map>

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
    void removeFromHeap(std::priority_queue<OrderDouble, std::vector<OrderDouble>, Compare> &heap, int id)
    { // come back here to make it more efficient
        std::vector<OrderDouble> temp;

        while(!heap.empty())
        {
            OrderDouble top = heap.top();
            
            heap.pop();
            
            if(top.id != id)
            {
                temp.emplace_back(top);
            }
        }

        for(const auto& order : temp)
        {
            heap.push(order);
        }
    }

public:
    void AddOrder(int id_, int quantity_, double price_, bool isBuyOrder_)
    {
        OrderDouble newOrder(id_, quantity_, price_, isBuyOrder_);
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

    void CancelOrder(int id)
    {
        if(orderMap.find(id) != orderMap.end())
        {
            OrderDouble cancelOrder = std::move(orderMap[id]);
            orderMap.erase(id);
            if (cancelOrder.isBuyOrder)
            {
                removeFromHeap(buyOrders, id);
            }
            else
            {
                removeFromHeap(sellOrders, id);
            }
        }
    }

    void modifyOrder(const int& id, const double& newPrice, const int& newQuantity)
    {
        if(orderMap.find(id) != orderMap.end())
        {
            Order oldOrder = orderMap[id];
            orderMap.erase(id);
            if (oldOrder.isBuyOrder)
            {
                removeFromHeap(buyOrders, id);
            }
            else
            {
                removeFromHeap(sellOrders, id);
            }
            AddOrder(id, newQuantity, newPrice, oldOrder.isBuyOrder);
        }
    }

    auto getBuyOrders() {return buyOrders;}
    auto getSellOrders() {return sellOrders;}
};