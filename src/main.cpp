#include <iostream>
#include <string>
#include <cstdlib>
#include <laserpants/dotenv/dotenv.h>
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

int main() {
    try {
        dotenv::init();
    } catch (const std::exception& e) {
        std::cerr << "Warning: Could not load .env file. " << e.what() << std::endl;
    }

    const char* env_p = std::getenv("API_KEY");
    if (!env_p) {
        std::cerr << "Error: API_KEY not found in environment." << std::endl;
        return 1;
    }
    std::string api_key = env_p;

    std::string symbol = "AAPL"; 
    std::string url = "https://finnhub.io/api/v1/quote?symbol=" + symbol + "&token=" + api_key;

    std::cout << "Fetching data for " << symbol << "..." << std::endl;

    cpr::Response r = cpr::Get(cpr::Url{url});

    if (r.status_code == 200) {
        json data = json::parse(r.text);
        if (data.contains("c") && data.contains("pc")) {
            double current_price = data["c"];
            double previous_close = data["pc"];
            double percent_change = ((current_price - previous_close) / previous_close) * 100.0;
            std::cout << "\n--- Stock Analysis: " << symbol << " ---" << std::endl;
            std::cout << "Current Price  : $" << current_price << std::endl;
            std::cout << "Previous Close : $" << previous_close << std::endl;
            std::cout << "Daily Change   : " << percent_change << "%" << std::endl;
            
            if (percent_change > 0) {
                std::cout << "Trend          : UP \xE2\x86\x91" << std::endl; 
            } else if (percent_change < 0) {
                std::cout << "Trend          : DOWN \xE2\x86\x93" << std::endl; 
            } else {
                std::cout << "Trend          : FLAT -" << std::endl;
            }

        } else {
            std::cerr << "Error: Unexpected JSON format." << std::endl;
            std::cerr << data.dump(4) << std::endl;
        }
    } else {
        std::cerr << "HTTP Error: " << r.status_code << std::endl;
        std::cerr << r.text << std::endl;
    }

    return 0;
}
