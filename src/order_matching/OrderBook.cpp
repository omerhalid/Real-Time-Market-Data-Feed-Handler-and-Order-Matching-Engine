#include "../include/order_matching/OrderBook.hpp"

template<typename Compare>
void OrderBook::removeFromHeap(std::priority_queue<OrderDouble, std::vector<OrderDouble>, Compare> &heap, int id)
{
    std::vector<OrderDouble> temp;

    while (!heap.empty())
    {
        OrderDouble top = heap.top();
        heap.pop();
        if (top.id != id)
        {
            temp.emplace_back(top);
        }
    }

    for (const auto& order : temp)
    {
        heap.push(order);
    }
}

void OrderBook::AddOrder(int id_, int quantity_, double price_, bool isBuyOrder_)
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

void OrderBook::CancelOrder(int id)
{
    if (orderMap.find(id) != orderMap.end())
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

void OrderBook::modifyOrder(const int& id, const double& newPrice, const int& newQuantity)
{
    if (orderMap.find(id) != orderMap.end())
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