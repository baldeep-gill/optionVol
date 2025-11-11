#include "contracts.h"
#include "HttpUtils.h"
#include "json.hpp"
#include <iostream>
#include <sstream>
#include "cache_utils.h"

float get_open_price(const std::string& date) {
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

std::map<long long, float> get_price(const std::string& date) {
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

        write_cache(cache_path, response);
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

std::vector<Contract> get_contracts(const std::string& underlying, const float& strike, const float& range, const std::string& type, const std::string& date) {
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

std::vector<VolumePoint> get_volume(const std::string& ticker, const std::string& date) {
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

        write_cache(cache_path, response);
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

std::vector<ContractVolumes> get_volume_par(ThreadPool& pool, const std::vector<Contract>& contracts, const std::string& date) {
    std::vector<std::future<ContractVolumes>> volume_futures;

    for (const auto& contract: contracts) {
        volume_futures.emplace_back(pool.enqueue([contract, date] {
            try {
                ContractVolumes cv;
                cv.strike = contract.strike;
                cv.slices = get_volume(contract.ticker, date);
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