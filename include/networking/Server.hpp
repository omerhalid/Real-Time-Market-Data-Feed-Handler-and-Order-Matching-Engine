#pragma once

#include <iostream>
#include <string>
#include <memory>
#include <thread>
#include "../boost_1_86_0/boost/asio.hpp"
#include "../json/json.hpp"
#include "../order_matching/OrderBook.hpp"
#include "../order_matching/Matcher.hpp"

using boost::asio::ip::tcp;

class Server {
public:
    Server(boost::asio::io_context& io_context, short port);

private:
    tcp::acceptor acceptor_;
    OrderBook orderBook;
    Matcher matcher;

    void acceptConnection();
    void handleClient(std::shared_ptr<tcp::socket> socket);
    std::string processRequest(const nlohmann::json& request);
    std::string handleMarketDataRequest(const nlohmann::json& request);
    std::string handleOrderRequest(const nlohmann::json& request);
};