#pragma once
#include <string>
#include <vector>
#include <map>
#include "thread_pool.h"

struct Contract {
    std::string ticker;
    double strike;
    std::string date;
    std::string type;
};

struct VolumePoint {
    long long timestamp;
    size_t volume;
};

struct ContractVolumes {
    double strike;
    std::vector<VolumePoint> slices;
};

float get_open_price(const std::string& date);

std::map<long long, float> get_price(const std::string& date);

std::vector<Contract> get_contracts(const std::string& underlying, const float& strike, const float& range, const std::string& type, const std::string& date);

std::vector<VolumePoint> get_volume(const std::string& ticker, const std::string& date);

std::vector<ContractVolumes> get_volume_par(ThreadPool& pool, const std::vector<Contract>& contracts, const std::string& date);

std::tuple<std::vector<long long>, std::vector<double>, std::vector<double>> calculate_aggregates(const std::string& underlying, const float& strike, const float& range, const std::string& date, ThreadPool& pool);
