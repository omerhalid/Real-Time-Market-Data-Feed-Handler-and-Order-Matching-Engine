#include <iostream>
#include <string>
#include <curl/curl.h>
#include "../json/json.hpp"

static std::string apiKey = ""; 

// Function to handle the response from cURL
size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// Function to fetch market data from Alpha Vantage
std::string fetchMarketData(const std::string& apiKey, const std::string& symbol) {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    curl = curl_easy_init();
    if(curl) {
        std::string url = "https://www.alphavantage.co/query?function=TIME_SERIES_INTRADAY&symbol=" + symbol + "&interval=5min&apikey=" + apiKey;
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }
    return readBuffer;
}

// Function to parse JSON response
void parseMarketData(const std::string& jsonData) {
    auto json = nlohmann::json::parse(jsonData);
    std::cout << json.dump(4) << std::endl; // Pretty print the JSON data
}

int main() {
    //std::string apiKey = "your_alpha_vantage_api_key"; // Replace with your actual API key
    std::string symbol = "AAPL"; // Replace with the stock symbol you are interested in

    std::string marketData = fetchMarketData(apiKey, symbol);
    parseMarketData(marketData);

    return 0;
}