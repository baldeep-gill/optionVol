#include "contracts.h"
#include "http_utils.h"
#include "thread_pool.h"
#include "json.hpp"
#include <iostream>
#include <sstream>

std::vector<Contract> get_contracts(const std::string& underlying, const float& strike, const float& range, const std::string& type, const std::string& date, const std::string& apiKey) {
    std::vector<Contract> contracts;

    std::ostringstream ss;
    ss  << "https://api.polygon.io/v3/reference/options/contracts?underlying_ticker=" << underlying << "&contract_type=" << type << "&expiration_date=" << date << "&as_of=" << date 
        << "&strike_price.gte=" << (strike * (1 - range)) << "&strike_price.lte=" << (strike * (1 + range)) << "&order=asc&limit=500&sort=strike_price&apiKey=" << apiKey;
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
            url += "&apiKey=" + apiKey;
        } else {
            url.clear();
        }
    }

    // std::cout << contracts.size() << " contracts fetched." << "\n";

    return contracts;
}

std::vector<VolumePoint> get_volume(const std::string& ticker, const std::string& date, const std::string& apiKey) {
    std::vector<VolumePoint> volumes;

    std::ostringstream ss;
    ss << "https://api.polygon.io/v2/aggs/ticker/" << ticker << "/range/5/minute/" << date << "/" << date << "?adjusted=true&sort=asc&apiKey=" << apiKey;
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
            point.timestamp = item.at("v");
            volumes.emplace_back( point );
        }
    }

    // std::cout << volumes.size() << " intervals fetched.\n";

    return volumes;
}