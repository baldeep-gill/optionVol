#include "contracts.h"
#include "http_utils.h"
#include "thread_pool.h"
#include "json.hpp"
#include <iostream>
#include <sstream>

std::vector<Contract> get_contracts(const std::string& underlying, const float& strike, const float& range, const std::string& type, const std::string& date) {
    std::vector<Contract> contracts;

    std::ostringstream ss;
    ss  << "https://api.polygon.io/v3/reference/options/contracts?underlying_ticker=" << underlying << "&contract_type=" << type << "&expiration_date=" << date << "&as_of=" << date 
        << "&strike_price.gte=" << (strike * (1 - range)) << "&strike_price.lte=" << (strike * (1 + range)) << "&order=asc&limit=500&sort=strike_price";
    std::string url = ss.str();

    // std::cout << "URL: " << url << "\n";

    while (!url.empty()) {
        std::string response = http_get(url);

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

    std::ostringstream ss;
    ss << "https://api.polygon.io/v2/aggs/ticker/" << ticker << "/range/5/minute/" << date << "/" << date << "?adjusted=true&sort=asc";
    std::string url = ss.str();

    std::string response = http_get(url);

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

    // std::cout << volumes.size() << " intervals fetched.\n";

    return volumes;
}

std::vector<ContractVolumes> get_volume_par(const std::vector<Contract>& contracts, size_t thread_count, const std::string& date) {
    ThreadPool pool(thread_count);
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