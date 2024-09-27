#pragma once

template <typename T>
struct Order
{
    int id;
    bool isBuyOrder;
    int quantity;
    T price;

    // Default constructor
    Order();
    // Constructor
    Order(int id_, bool isBuyOrder_, int quantity_, T price_);
};

// Default constructor definition
template <typename T>
Order<T>::Order() : 
    id(0), isBuyOrder(0), quantity(0), price(0) {}

// Constructor definition
template <typename T>
Order<T>::Order(int id_, bool isBuyOrder_, int quantity_, T price_) : 
    id(id_), isBuyOrder(isBuyOrder_), quantity(quantity_), price(price_) {}