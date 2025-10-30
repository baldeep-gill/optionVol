#include <iostream>
#include "contracts.h"
#include "http_utils.h"
#include "thread_pool.h"
#include <cstdlib>
#include <chrono>
#include <ctime>
#include <matplot/matplot.h>

std::string epoch_to_timestamp(long long epoch) {
    epoch -= 5 * 3600 * 1000;
    std::time_t t = static_cast<std::time_t>(epoch / 1000);
    std::tm tm = *std::localtime(&t);

    char buffer[30];
    std::strftime(buffer, sizeof(buffer), "%H:%M", &tm);
    return std::string(buffer);
}

int main() {
    const size_t thread_count = 30;
    ThreadPool pool(thread_count);

    std::string underlying = "SPX";
    float strike = 6700;
    float range = 0.15;
    std::string date = "2025-10-22";

    std::cout << "Enter date (YYYY-MM-DD): ";
    std::cin >> date;
    std::cin.clear();
    std::cin.ignore(10000, '\n');

    init_curl();

    std::vector<Contract> call_contracts = get_contracts(underlying, strike, range, "call", date);
    std::vector<Contract> put_contracts = get_contracts(underlying, strike, range, "put", date);
        
    auto start = std::chrono::high_resolution_clock::now();

    std::vector<ContractVolumes> call_volumes = get_volume_par(pool, call_contracts, date);
    std::vector<ContractVolumes> put_volumes = get_volume_par(pool, put_contracts, date);
    std::cout << "Fetched volumes for " << call_volumes.size() + put_volumes.size() << " contracts.\n";

    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
    std::cout << "Took " << duration.count() << " milliseconds.\n";

    std::map<long long, float> call_acc;         // For each timestamp store strike_t * vol_s_t
    std::map<long long, size_t> call_vol_aggs;   // For each timestamp store vol_t

    std::map<long long, float> put_acc;
    std::map<long long, size_t> put_vol_aggs;

    for (ContractVolumes& vec: call_volumes) {
        for (VolumePoint& item: vec.slices) {
            call_acc[item.timestamp] += vec.strike * item.volume;
            call_vol_aggs[item.timestamp] += item.volume;
        }
    }

    for (ContractVolumes& vec: put_volumes) {
        for (VolumePoint& item: vec.slices) {
            put_acc[item.timestamp] += vec.strike * item.volume;
            put_vol_aggs[item.timestamp] += item.volume;
        }
    }

    // std::cout << "CALLS:\n";
    // for (auto& entry: call_acc) {
    //     float temp = entry.second / call_vol_aggs[entry.first];
    //     entry.second = temp;
    //     std::cout << epoch_to_timestamp(entry.first) << ": " << temp << "\n";
    // }

    // std::cout << "PUTS:\n";
    // for (auto& entry: put_acc) {
    //     float temp = entry.second / put_vol_aggs[entry.first];
    //     entry.second = temp;
    //     std::cout << epoch_to_timestamp(entry.first) << ": " << temp << "\n";
    // }

    std::vector<double> call_vwas, put_vwas;
    for (auto& [ts, acc]: call_acc) {
        if (call_vol_aggs[ts] > 0) {
            call_vwas.push_back(acc / call_vol_aggs[ts]);
            put_vwas.push_back(put_acc.count(ts) && put_vol_aggs[ts] > 0 ? put_acc[ts] / put_vol_aggs[ts] : NAN);
        }
    }

    cleanup_curl();
    matplot::plot(call_vwas, "-g")->line_width(2).display_name("Calls");
    matplot::hold(matplot::on);
    matplot::plot(put_vwas, "-r")->line_width(2).display_name("Puts");

    matplot::show();

    return 0;
}