#include "contracts.h"
#include "http_utils.h"
#include <iostream>
#include <sstream>

std::vector<Contract> get_contracts(const std::string& underlying, const float& strike, const float& range, const std::string& type, const std::string& date, const std::string& apiKey) {
    std::vector<Contract> contracts;

    std::ostringstream ss;
    ss  << "https://api.polygon.io/v3/reference/options/contracts?underlying_ticker=" << underlying << "&contract_type=" << type << "&expiration_date=" << date << "&as_of=" << date 
        << "&strike_price.gte=" << (strike * (1 - range)) << "&strike_price_lte=" << (strike * (1 + range)) << "&order=asc&limit=15&sort=strike_price&apiKey=" << apiKey;
    std::string url = ss.str();

    std::string response = http_get(url);

    if (response.empty()) {
        std::cerr << "Empty response from Polygon\n";
        return contracts;
    }

    std::cout << response << "\n";

    // TODO: Parse json into contracts vector

    return contracts;
}