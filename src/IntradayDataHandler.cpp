#include "IntradayDataHandler.h"
#include "HttpUtils.h"
#include "json.hpp"
#include <chrono>
#include <iostream>
#include <sstream>

void IntradayDataHandler::do_work(const std::string& underlying, const float& strike, const float& range) {
    while (true) {
        IntradayDataHandler::calculate_aggregates(underlying, strike, range);

        std::cout << "Fetched new data. Total size: " << IntradayDataHandler::aggregates.timestamps.size() << "\n";

        IntradayDataHandler::visHandle.drawOverall(underlying, IntradayDataHandler::date);

        std::this_thread::sleep_for(std::chrono::minutes(5));
    }
}

void IntradayDataHandler::calculate_aggregates(const std::string& underlying, const float& strike, const float& range) {
    DataAggregates agg;
    std::vector<long long> timestamps;
    std::vector<double> spot;
    std::vector<double> call_vwas;
    std::vector<double> put_vwas;
    
    std::ostringstream ss;
    ss << underlying << "_" << strike << "_" << range << "_" << date;
    std::string response;
    
    auto start = std::chrono::high_resolution_clock::now();


    // TODO: These only need to be called once per day
    std::vector<Contract> call_contracts = DataHandler::get_contracts(underlying, strike, range, "call", date);
    std::vector<Contract> put_contracts = DataHandler::get_contracts(underlying, strike, range, "put", date);    

    std::vector<ContractVolumes> call_volumes = IntradayDataHandler::get_volume_par(call_contracts);
    std::vector<ContractVolumes> put_volumes = IntradayDataHandler::get_volume_par(put_contracts);
    
    std::map<long long, float> spx_price = IntradayDataHandler::get_price();
    
    
    std::cout << "Fetched volumes for " << call_volumes.size() + put_volumes.size() << " contracts.\n";
    
    std::map<long long, double> call_accumulator;         // For each timestamp store strike_t * vol_s_t
    std::map<long long, size_t> call_vol_aggregates;   // For each timestamp store vol_t
    
    for (ContractVolumes& vec: call_volumes) {
        for (VolumePoint& item: vec.slices) {
            call_accumulator[item.timestamp] += vec.strike * item.volume;
            call_vol_aggregates[item.timestamp] += item.volume;
        }
    }
    
    std::map<long long, double> put_accumulator;
    std::map<long long, size_t> put_vol_aggregates;
    
    for (ContractVolumes& vec: put_volumes) {
        for (VolumePoint& item: vec.slices) {
            put_accumulator[item.timestamp] += vec.strike * item.volume;
            put_vol_aggregates[item.timestamp] += item.volume;
        }
    }
    
    for (auto& [ts, acc]: call_accumulator) {
        timestamps.push_back(ts);
        
        call_vwas.push_back( 
            call_vol_aggregates[ts] > 0
            ? acc / call_vol_aggregates[ts]
            : 0
        );
        
        put_vwas.push_back(
            put_accumulator.count(ts) && put_vol_aggregates[ts] > 0
            ? put_accumulator[ts] / put_vol_aggregates[ts]
            : 0
        );
        
        spot.push_back(spx_price.count(ts) ? spx_price[ts] : NAN);
    }
    
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
    std::cout << "Took " << duration.count() << " milliseconds.\n";
    
    agg.underlying = underlying;
    agg.date = date;
    agg.timestamps = timestamps;
    agg.spot = spot;
    agg.calls = call_vwas;
    agg.puts = put_vwas;
    
    IntradayDataHandler::update_last_fetch(agg.timestamps.back());
    IntradayDataHandler::update_aggregates(std::move(agg));
}

void IntradayDataHandler::update_aggregates(DataAggregates&& aggs) {
    std::move(aggs.timestamps.begin(), aggs.timestamps.end(), std::back_inserter(IntradayDataHandler::aggregates.timestamps));
    std::move(aggs.calls.begin(), aggs.calls.end(), std::back_inserter(IntradayDataHandler::aggregates.calls));
    std::move(aggs.puts.begin(), aggs.puts.end(), std::back_inserter(IntradayDataHandler::aggregates.puts));
    std::move(aggs.spot.begin(), aggs.spot.end(), std::back_inserter(IntradayDataHandler::aggregates.spot));

    aggs.timestamps.clear();
    aggs.calls.clear();
    aggs.puts.clear();
    aggs.spot.clear();

    IntradayDataHandler::visHandle.updateData(IntradayDataHandler::aggregates);
}

void IntradayDataHandler::update_last_fetch(long long last) {
    // auto now = std::chrono::system_clock::now();
    // auto epoch = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

    last_fetch = last + (5 * 60 * 1000);
}

std::map<long long, float> IntradayDataHandler::get_price() {
    std::map<long long, float> price;
    std::string response;
    
    std::ostringstream ss;
    ss << "https://api.massive.com/v2/aggs/ticker/I:SPX/range/5/minute/" << last_fetch << "/" << IntradayDataHandler::date << "?sort=asc";
    std::string url = ss.str();

    response = HttpUtils::http_get(url);

    if (response.empty()) {
        std::cerr << "Empty response from Polygon (get_price)\n";
        return price;
    }


    nlohmann::json data = nlohmann::json::parse(response, nullptr, false);
    if (data.is_discarded()) {
        std::cerr << "Failed to parse JSON response (get_price)\n";
        return price;
    }

    if (data.contains("results") && data["results"].is_array()) {
        for (auto& item: data["results"]) {
            price[item.at("t")] = item.at("c");
        }
    }

    return price;
}

std::vector<VolumePoint> IntradayDataHandler::get_volume(const std::string& ticker)
{
    std::vector<VolumePoint> volumes;
    std::string response;

    std::ostringstream ss;
    ss << "https://api.massive.com/v2/aggs/ticker/" << ticker << "/range/5/minute/" << last_fetch << "/" << IntradayDataHandler::date << "?adjusted=true&sort=asc";
    std::string url = ss.str();

    response = HttpUtils::http_get(url);

    if (response.empty()) {
        std::cerr << "Empty response from Polygon (get_volume)\n";
        return volumes;
    }

    nlohmann::json data = nlohmann::json::parse(response, nullptr, false);
    if (data.is_discarded()) {
        std::cerr << "Failed to parse JSON response (get_volume)\n";
        return volumes;
    }

    if (data.contains("results") && data["results"].is_array()) {
        for (auto& item: data["results"]) {
            VolumePoint point;
            point.timestamp = item.at("t");
            point.volume = item.at("v");
            volumes.emplace_back( point );
        }
    }

    return volumes;
}

std::vector<ContractVolumes> IntradayDataHandler::get_volume_par(const std::vector<Contract>& contracts)
{
    std::vector<std::future<ContractVolumes>> volume_futures;

    for (const auto& contract: contracts) {
        volume_futures.emplace_back(DataHandler::threadPool.enqueue([this, contract] {
            try {
                ContractVolumes cv;
                cv.strike = contract.strike;
                cv.slices = IntradayDataHandler::get_volume(contract.ticker);
                return cv;
            } catch (...) {
                return ContractVolumes{ contract.strike, {} };
            }
        }));
    }

    std::vector<ContractVolumes> volumes;
    volumes.reserve(volume_futures.size());

    for (auto& f: volume_futures) {
        volumes.push_back(f.get());
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    return volumes;
}
