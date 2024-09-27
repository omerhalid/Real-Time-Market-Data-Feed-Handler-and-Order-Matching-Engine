#include "Client.cpp"
#include <iostream>
#include <nlohmann/json.hpp>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: client <server_address> <symbol>\n";
        return 1;
    }

    std::string serverAddress = argv[1];
    std::string symbol = argv[2];

    Client client;

    // Fetch market data
    std::string apiKey = std::getenv("API_KEY");
    std::string marketData = client.fetchMarketData(apiKey, symbol);
    std::cout << "Fetched Market Data: " << marketData << "\n";

    // You can also send orders to the server, e.g., a buy order
    nlohmann::json orderRequest = {
        {"type", "place_order"},
        {"order_id", 1},
        {"is_buy_order", true},
        {"quantity", 10},
        {"price", 145.0},
        {"symbol", symbol}
    };
    std::string response = client.sendRequestToServer(serverAddress, orderRequest);
    std::cout << "Server response: " << response << "\n";

    return 0;
}
