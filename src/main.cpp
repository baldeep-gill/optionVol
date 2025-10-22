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
    
    std::vector<Contract> contracts = get_contracts(underlying, strike, range, "call", date, apiKey);
    
    // for (auto& item: contracts) {
        //     std::cout << item.ticker << "\n";
        // }
        
    std::map<long long, float> acc;
    std::map<long long, size_t> vol_aggs;

    for (Contract& contract: contracts) {
        int strike = contract.strike;
        std::vector<std::pair<long long, size_t>> vols = get_volume(contract.ticker, date, apiKey);

        for (std::pair<long long, size_t>& item: vols) {
            acc[item.first] += strike * item.second;
            vol_aggs[item.first] += item.second;
        }

        std::cout << strike << "\n";
    }

    for (auto& entry: acc) {
        float temp = entry.second / vol_aggs[entry.first];
        entry.second = temp;
        std::cout << entry.first << ": " << temp << "\n";
    }

    cleanup_curl();

    return 0;
}