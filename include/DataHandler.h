#pragma once
#include <string>
#include <vector>
#include <map>
#include "contracts.h"
#include "thread_pool.h"

class DataHandler {
    public:
        DataHandler(size_t thread_count) : threadPool(thread_count) {};
        float get_open_price(const std::string& date);
        std::map<long long, float> get_price(const std::string& date);
        DataAggregates calculate_aggregates(const std::string& underlying, const float& strike, const float& range, const std::string& date);
        
    private:
        ThreadPool threadPool;
        std::vector<Contract> get_contracts(const std::string& underlying, const float& strike, const float& range, const std::string& type, const std::string& date);
        std::vector<VolumePoint> get_volume(const std::string& ticker, const std::string& date);
        std::vector<ContractVolumes> get_volume_par(const std::vector<Contract>& contracts, const std::string& date);
};