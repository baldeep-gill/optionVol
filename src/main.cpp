#include <iostream>
#include "contracts.h"
#include "http_utils.h"
#include <cstdlib>
#include <chrono>

int main() {
    std::string underlying = "SPX";
    float strike = 6700;
    float range = 0.05;
    std::string date = "2025-10-10";

    init_curl();

    std::vector<Contract> contracts = get_contracts(underlying, strike, range, "call", date);
        
    auto start = std::chrono::high_resolution_clock::now();

    std::vector<ContractVolumes> all_volumes = get_volume_par(contracts, 30, date);
    std::cout << "Fetched volumes for " << all_volumes.size() << " contracts.\n";

    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
    std::cout << "Took " << duration.count() << " milliseconds.\n";

    std::map<long long, float> acc;         // For each timestamp store strike_t * vol_s_t
    std::map<long long, size_t> vol_aggs;   // For each timestamp store vol_t

    for (ContractVolumes& vec: all_volumes) {
        for (VolumePoint& item: vec.slices) {
            acc[item.timestamp] += vec.strike * item.volume;
            vol_aggs[item.timestamp] += item.volume;
        }
    }

    for (auto& entry: acc) {
        float temp = entry.second / vol_aggs[entry.first];
        entry.second = temp;
        std::cout << entry.first << ": " << temp << "\n";
    }

    cleanup_curl();

    return 0;
}