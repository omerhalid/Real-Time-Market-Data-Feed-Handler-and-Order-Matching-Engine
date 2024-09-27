#include <iostream>
#include "Client.cpp"  // Use the Client class
#include "../json/json.hpp"
#include <string>

class MarketDataHandler {
public:
    MarketDataHandler() = default;

    // Fetch and process market data using the Client class
    void updateMarketData(const std::string& symbol) {
        Client client;
        std::string apiKey = std::getenv("ALPHA_VANTAGE_API_KEY");
        std::string marketDataJson = client.fetchMarketData(apiKey, symbol); // Fetch using Client

        if (!marketDataJson.empty()) {
            parseMarketData(marketDataJson);  // Process the fetched data
        }
    }

private:
    // Parse the JSON market data
    void parseMarketData(const std::string& jsonData) {
        try {
            auto json = nlohmann::json::parse(jsonData);
            std::cout << "Processing market data...\n";
            
            // Extract specific fields and update your system
            std::string symbol = json["Meta Data"]["2. Symbol"];
            double closePrice = json["Time Series (5min)"]["2023-09-22 16:00:00"]["4. close"];
            
            // Update internal data structures here (e.g., order book, notifications)
            std::cout << "Symbol: " << symbol << ", Close Price: " << closePrice << std::endl;

        } catch (const nlohmann::json::parse_error& e) {
            std::cerr << "JSON parse error: " << e.what() << std::endl;
        }
    }
};
