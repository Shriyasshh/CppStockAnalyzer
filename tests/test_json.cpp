#include <iostream>
#include <string>
#include <nlohmann/json.hpp> 

int main() {
    std::string raw_json_data = R"(
        {
            "company": "Tech Corp",
            "ticker": "TCRP",
            "price": 150.75,
            "is_market_open": true,
            "recent_prices": [148.50, 149.20, 150.75]
        }
    )";

    std::cout << "Attempting to parse JSON...\n";

    try {
        nlohmann::json stock_data = nlohmann::json::parse(raw_json_data);

        std::string ticker = stock_data["ticker"];
        double current_price = stock_data["price"];
        double oldest_price = stock_data["recent_prices"][0];

        std::cout << "Success! Here is the parsed data:\n";
        std::cout << "---------------------------------\n";
        std::cout << "Stock: " << ticker << "\n";
        std::cout << "Current Price: $" << current_price << "\n";
        std::cout << "Oldest Tracked Price: $" << oldest_price << "\n";

    } catch (const nlohmann::json::parse_error& e) {
        std::cerr << "JSON parsing failed: " << e.what() << '\n';
    }

    return 0;
}
