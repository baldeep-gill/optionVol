#pragma once
#include "DataHandler.h"

class IntradayDataHandler : public DataHandler {
    public:
        IntradayDataHandler(size_t thread_count, const std::string& date) : DataHandler(thread_count), date(date) {};
        DataAggregates calculate_aggregates(const std::string& underlying, const float& strike, const float& range);
        
    private:
        std::string date;
        long long last_fetch;
        void update_last_fetch();
        std::map<long long, float> get_price();
        std::vector<VolumePoint> get_volume(const std::string& ticker);
        std::vector<ContractVolumes> get_volume_par(const std::vector<Contract>& contracts); 
};