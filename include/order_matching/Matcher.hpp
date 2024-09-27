#pragma once

#include "OrderBook.hpp"

class Matcher
{
public:
    void MatchOrders(OrderBook& orderBook);
};