#pragma once

#include <iostream>
#include "Client.hpp"  // Use the Client class
#include "../json/json.hpp"
#include <string>

class MarketDataHandler {
public:
    MarketDataHandler() = default;

    // Fetch and process market data using the Client class
    void updateMarketData(const std::string& symbol);

private:
    // Parse the JSON market data
    void parseMarketData(const std::string& jsonData);
};
