#pragma once
#include <string>
#include <vector>

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

struct DataAggregates {
    std::string underlying;
    std::string date;
    std::vector<long long> timestamps;
    std::vector<double> spot;
    std::vector<double> calls;
    std::vector<double> puts;
};
