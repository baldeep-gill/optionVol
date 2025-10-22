#ifndef CONTRACTS_H
#define CONTRACTS_H

#include <string>
#include <vector>
#include <map>

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

std::vector<Contract> get_contracts(const std::string& underlying, const float& strike, const float& range, const std::string& type, const std::string& date);

std::vector<VolumePoint> get_volume(const std::string& ticker, const std::string& date);

std::vector<ContractVolumes> get_volume_par(const std::vector<Contract>& contracts, size_t thread_count, const std::string& date);

#endif