#pragma once
#include "DataHandler.h"
#include "VisHandle.h"
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <iostream>

class IntradayDataHandler : public DataHandler {
    public:
        IntradayDataHandler(const std::string underlying="SPX", const float& range=0.15) : DataHandler{30}, visHandle{}, underlying{underlying}, range{range} {
            auto now = std::chrono::system_clock::now();
            std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
            std::tm now_tm;

            localtime_r(&now_time_t, &now_tm);

            now_tm.tm_hour = 0;
            now_tm.tm_min = 0;
            now_tm.tm_sec = 0;
            
            // Set last_fetch to midnight (start of day)
            last_fetch = mktime(&now_tm);

            // Get today's date
            char buffer[std::size("YYYY-MM-DD")];
            std::strftime(buffer, sizeof(buffer), "%Y-%m-%d", &now_tm);

            date = buffer;

            
            open_price = DataHandler::get_open_price(date);
            
            call_contracts = DataHandler::get_contracts(underlying, open_price, 0.15, "call", date);
            put_contracts = DataHandler::get_contracts(underlying, open_price, 0.15, "put", date);    
        };
        void calculate_aggregates();
        void do_work();
        
    private:
        std::string date, underlying;
        float open_price, range;
        long long last_fetch;
        DataAggregates aggregates;
        VisHandle visHandle;
        std::vector<Contract> call_contracts, put_contracts;

        void update_aggregates(DataAggregates&& aggs);
        void update_last_fetch(long long& last);
        std::map<long long, float> get_price();
        std::vector<VolumePoint> get_volume(const std::string& ticker);
        std::vector<ContractVolumes> get_volume_par(const std::vector<Contract>& contracts); 
};