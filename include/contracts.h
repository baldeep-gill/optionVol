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

std::vector<Contract> get_contracts(const std::string& underlying, const float& strike, const float& range, const std::string& type, const std::string& date, const std::string& apiKey);

std::vector<VolumePoint> get_volume(const std::string& ticker, const std::string& date, const std::string& apiKey);

#endif