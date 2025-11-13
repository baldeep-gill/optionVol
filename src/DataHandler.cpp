#include "DataHandler.h"
#include "contracts.h"
#include "HttpUtils.h"
#include "cache_utils.h"
#include "json.hpp"
#include <iostream>
#include <sstream>

float DataHandler::get_open_price(const std::string& date) {
    std::ostringstream ss;
    ss << "https://api.massive.com/v2/aggs/ticker/I:SPX/range/1/day/" << date << "/" << date << "?sort=asc";
    std::string url = ss.str();
    
    std::string response = HttpUtils::http_get(url);

    if (response.empty()) {
        std::cerr << "Empty response from Polygon (get_volume)\n";
        return 0.0;
    }

    nlohmann::json data = nlohmann::json::parse(response, nullptr, false);
    if (data.is_discarded()) {
        std::cerr << "Failed to parse JSON response (get_volume)\n";
        return 0.0;
    }

    if (data.contains("results") && data["results"].is_array()) {
        for (auto& item: data["results"]) {
            return item.at("o");
        }
    }

    return 0.0;
}

std::map<long long, float> DataHandler::get_price(const std::string& date) {
    std::map<long long, float> price;
    std::string cache_path = generate_filename("price", date);
    std::string response;

    if (cache_exists(cache_path)) {
        response = read_cache(cache_path).value();
    } else {
        std::ostringstream ss;
        ss << "https://api.massive.com/v2/aggs/ticker/I:SPX/range/5/minute/" << date << "/" << date << "?sort=asc";
        std::string url = ss.str();

        response = HttpUtils::http_get(url);

        if (response.empty()) {
            std::cerr << "Empty response from Polygon (get_price)\n";
            return price;
        }

        write_cache(cache_path, response, DataHandler::threadPool);
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

std::vector<Contract> DataHandler::get_contracts(const std::string& underlying, const float& strike, const float& range, const std::string& type, const std::string& date) {
    std::vector<Contract> contracts;

    std::ostringstream ss;
    ss  << "https://api.massive.com/v3/reference/options/contracts?underlying_ticker=" << underlying << "&contract_type=" << type << "&expiration_date=" << date << "&as_of=" << date 
        << "&strike_price.gte=" << (strike * (1 - range)) << "&strike_price.lte=" << (strike * (1 + range)) << "&order=asc&sort=strike_price";
    std::string url = ss.str();

    // std::cout << "URL: " << url << "\n";

    while (!url.empty()) {
        std::string response = HttpUtils::http_get(url);

        if (response.empty()) {
            std::cerr << "Empty response from Polygon (get_contracts)\n";
            return contracts;
        }

        nlohmann::json data = nlohmann::json::parse(response, nullptr, false);
        if (data.is_discarded()) {
            std::cerr << "Failed to parse JSON response (get_contracts)\n";
            return contracts;
        }

        if (data.contains("results") && data["results"].is_array()) {
            for (auto& item : data["results"]) {
                Contract contract;
                contract.ticker = item.at("ticker");
                contract.strike = item.at("strike_price");
                contract.date = item.at("expiration_date");
                contract.type = item.at("contract_type");

                contracts.push_back(contract);
            }
        }

        if (data.contains("next_url") && !data["next_url"].is_null()) {
            url = data["next_url"];
        } else {
            url.clear();
        }
    }

    // std::cout << contracts.size() << " contracts fetched." << "\n";

    return contracts;
}

std::vector<VolumePoint> DataHandler::get_volume(const std::string& ticker, const std::string& date) {
    std::vector<VolumePoint> volumes;
    std::string cache_path = generate_filename("contract", ticker);
    std::string response;

    if (cache_exists(cache_path)) {
        response = read_cache(cache_path).value();
    } else {
        std::ostringstream ss;
        ss << "https://api.massive.com/v2/aggs/ticker/" << ticker << "/range/5/minute/" << date << "/" << date << "?adjusted=true&sort=asc";
        std::string url = ss.str();

        response = HttpUtils::http_get(url);

        if (response.empty()) {
            std::cerr << "Empty response from Polygon (get_volume)\n";
            return volumes;
        }

        write_cache(cache_path, response, DataHandler::threadPool);
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

    // std::cout << volumes.size() << " intervals fetched.\n";

    return volumes;
}

std::vector<ContractVolumes> DataHandler::get_volume_par(const std::vector<Contract>& contracts, const std::string& date) {
    std::vector<std::future<ContractVolumes>> volume_futures;

    for (const auto& contract: contracts) {
        volume_futures.emplace_back(DataHandler::threadPool.enqueue([this, contract, date] {
            try {
                ContractVolumes cv;
                cv.strike = contract.strike;
                cv.slices = DataHandler::get_volume(contract.ticker, date);
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

DataAggregates DataHandler::calculate_aggregates(const std::string& underlying, const float& strike, const float& range, const std::string& date) {
    DataAggregates agg;
    std::vector<long long> timestamps;
    std::vector<double> spot;
    std::vector<double> call_vwas;
    std::vector<double> put_vwas;
    
    std::ostringstream ss;
    ss << underlying << "_" << strike << "_" << range << "_" << date;
    std::string cache_path = generate_filename("aggregate", ss.str());
    std::string response;
    
    auto start = std::chrono::high_resolution_clock::now();

    if (cache_exists(cache_path)) {
        response = read_cache(cache_path).value();

        nlohmann::json data = nlohmann::json::parse(response, nullptr, false);
        if (data.is_discarded()) {
            std::cerr << "Failed to parse JSON response (get_volume)\n";
            return agg;
        }

        if (data.contains("timestamps") && data["timestamps"].is_array()) {
            for (auto& item: data["timestamps"]) {
                timestamps.emplace_back( item );
            }
        }

        if (data.contains("spot") && data["spot"].is_array()) {
            for (auto& item: data["spot"]) {
                spot.emplace_back( item );
            }
        }

        if (data.contains("calls") && data["calls"].is_array()) {
            for (auto& item: data["calls"]) {
                call_vwas.emplace_back(item);
            }
        }

        if (data.contains("puts") && data["puts"].is_array()) {
            for (auto& item: data["puts"]) {
                put_vwas.emplace_back(item);
            }
        }
    } else {
        std::vector<Contract> call_contracts = DataHandler::get_contracts(underlying, strike, range, "call", date);
        std::vector<Contract> put_contracts = DataHandler::get_contracts(underlying, strike, range, "put", date);    

        std::vector<ContractVolumes> call_volumes = DataHandler::get_volume_par(call_contracts, date);
        std::vector<ContractVolumes> put_volumes = DataHandler::get_volume_par(put_contracts, date);

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
        }


        std::map<long long, float> spx_price = DataHandler::get_price(date);
        for (auto& ts: timestamps) spot.push_back(spx_price.count(ts) ? spx_price[ts] : NAN);

        nlohmann::json j;
        j["timestamps"] = timestamps;
        j["spot"] = spot;
        j["calls"] = call_vwas;
        j["puts"] = put_vwas;

        write_cache(cache_path, j.dump(), DataHandler::threadPool);
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

    return agg;
}