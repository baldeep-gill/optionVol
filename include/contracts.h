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

std::vector<Contract> get_contracts(const std::string& underlying, const float& strike, const float& range, const std::string& type, const std::string& date, const std::string& apiKey);

std::map<long long, int> get_volume(const std::string& ticker, const std::string& date, const std::string& apiKey);

#endif