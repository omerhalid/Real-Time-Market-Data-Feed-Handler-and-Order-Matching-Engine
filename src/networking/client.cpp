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
    void loadEnv() {
        std::ifstream envFile("../../.env");
        if (!envFile.is_open()) {
            std::cerr << "Failed to open .env file" << std::endl;
            return;
        }

        std::string line;
        while (std::getline(envFile, line)) {
            std::istringstream is_line(line);
            std::string key;
            if (std::getline(is_line, key, '=')) {
                std::string value;
                if (std::getline(is_line, value)) {
                    #ifdef _WIN32
                        _putenv_s(key.c_str(), value.c_str());
                    #else
                        setenv(key.c_str(), value.c_str(), 1);
                    #endif
                }
            }
        }
        envFile.close();
    }

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
            curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);  // Timeout
            // Disable SSL verification (not recommended for production)
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
            res = curl_easy_perform(curl);
            if(res != CURLE_OK) {
                std::cerr << "cURL error: " << curl_easy_strerror(res) << std::endl;
            }
            curl_easy_cleanup(curl);
        } else {
            std::cerr << "Failed to initialize cURL" << std::endl;
        }

        // Print the response for debugging
        std::cout << "Response: " << readBuffer << std::endl;

        return readBuffer;
    }

    // Function to parse JSON response
    void parseMarketData(const std::string& jsonData) {
        try {
            auto json = nlohmann::json::parse(jsonData);
            std::cout << json.dump(4) << std::endl; // Pretty print the JSON data

            // Example: Parsing specific fields
            std::string symbol = json["Meta Data"]["2. Symbol"];
            double closePrice = json["Time Series (5min)"]["2023-09-22 16:00:00"]["4. close"];
            std::cout << "Symbol: " << symbol << ", Close Price: " << closePrice << std::endl;

        } catch (const nlohmann::json::parse_error& e) {
            std::cerr << "JSON parse error: " << e.what() << std::endl;
        }
    }
};
