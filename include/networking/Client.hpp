#pragma once

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <curl/curl.h>
#include "../json/json.hpp"
#include <stdlib.h>

class Client
{
public:
    // Function to load environment variables from a .env file
    void loadEnv();

    // Function to handle the response from cURL
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);

    // Function to fetch market data from Alpha Vantage
    std::string fetchMarketData(const std::string& apiKey, const std::string& symbol);

    // Function to send a request to the server
    std::string sendRequestToServer(const std::string& serverAddress, const nlohmann::json& requestJson);
};