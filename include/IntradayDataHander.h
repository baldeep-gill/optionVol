#pragma once
#include "DataHandler.h"

class IntradayDataHandler : public DataHandler {
    public:
        IntradayDataHandler(size_t thread_count) : DataHandler(thread_count) {};
        DataAggregates calculate_aggregates(const std::string& underlying, const float& strike, const float& range);
        
    private:
        long long last_fetch;
        void update_last_fetch();
        std::map<long long, float> get_price(const std::string& date);
        std::vector<Contract> get_contracts(const std::string& underlying, const float& strike, const float& range, const std::string& type);
        std::vector<VolumePoint> get_volume(const std::string& ticker);
        std::vector<ContractVolumes> get_volume_par(const std::vector<Contract>& contracts); 
};