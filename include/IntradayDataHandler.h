#pragma once
#include "DataHandler.h"
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <iostream>

class IntradayDataHandler : public DataHandler {
    public:
        IntradayDataHandler() : DataHandler(30) {
            auto now = std::chrono::system_clock::now();
            std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
            std::tm now_tm;

            localtime_r(&now_time_t, &now_tm);

            // Set last_fetch to midnight (start of day)
            now_tm.tm_hour = 0;
            now_tm.tm_min = 0;
            now_tm.tm_sec = 0;

            last_fetch = mktime(&now_tm);
            
            // Get today's date
            char buffer[std::size("YYYY-MM-DD")];
            std::strftime(buffer, sizeof(buffer), "%Y-%m-%d", &now_tm);

            date = buffer;
        };
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