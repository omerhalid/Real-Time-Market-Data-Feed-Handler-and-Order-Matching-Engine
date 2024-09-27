#include "../include/order_matching/Matcher.hpp"
#include <iostream>
#include <algorithm> // For std::min

void Matcher::MatchOrders(OrderBook& orderBook)
{
    while (!orderBook.getBuyOrders().empty() && !orderBook.getSellOrders().empty())
    {
        OrderDouble buyOrder = orderBook.getBuyOrders().top();
        OrderDouble sellOrder = orderBook.getSellOrders().top();

        // Check if there is a match
        if (buyOrder.price >= sellOrder.price)
        {
            int matchedQuantity = std::min(buyOrder.quantity, sellOrder.quantity);

            // Execute the trade
            std::cout << "Trade executed: " << matchedQuantity << " units at price " << sellOrder.price << std::endl;

            // Adjust the quantities of the buy and sell orders
            buyOrder.quantity -= matchedQuantity;
            sellOrder.quantity -= matchedQuantity;

            // Remove the orders from the heaps
            orderBook.getBuyOrders().pop();
            orderBook.getSellOrders().pop();

            // If there is remaining quantity, push the order back into the heap
            if (buyOrder.quantity > 0)
            {
                orderBook.getBuyOrders().push(buyOrder);
            }
            if (sellOrder.quantity > 0)
            {
                orderBook.getSellOrders().push(sellOrder);
            }
        }
        else
        {
            // No match found, exit the loop
            break;
        }
    }
}