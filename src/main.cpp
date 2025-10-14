#include <iostream>
#include "contracts.h"
#include "http_utils.h"
#include <cstdlib>

int main() {
    std::string apiKey = std::getenv("POLYGON_API_KEY");
    std::string underlying = "SPX";
    float strike = 6700;
    float range = 0.05;
    std::string date = "2025-10-10";

    init_curl();

    std::vector<Contract> contracts = get_contracts(underlying, strike, range, "call", date, apiKey);

    cleanup_curl();

    // for (auto& item: contracts) {
    //     std::cout << item.ticker << "\n";
    // }

    return 0;
}