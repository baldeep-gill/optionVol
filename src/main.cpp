#include "contracts.h"
#include "thread_pool.h"
#include "HttpUtils.h"
#include "VisHandle.h"
#include "DataHandler.h"
#include "IntradayDataHandler.h"

std::string epoch_to_timestamp(long long epoch) {
    epoch -= 5 * 3600 * 1000;
    std::time_t t = static_cast<std::time_t>(epoch / 1000);
    std::tm tm = *std::localtime(&t);

    char buffer[30];
    std::strftime(buffer, sizeof(buffer), "%H:%M", &tm);
    return std::string(buffer);
}

int main() {
    const size_t thread_count = 30;
    static DataHandler dataHandler(thread_count);
    static IntradayDataHandler intraDataHandler;

    std::string underlying = "SPX";
    std::string date = "2025-10-22";
    float strike = dataHandler.get_open_price(date);
    float range = 0.15;

    DataAggregates aggs = dataHandler.calculate_aggregates(underlying, strike, range, date);

    VisHandle visHandle(aggs);
    visHandle.drawOverall(underlying, date);
    // visHandle.drawStepped(underlying, date, 250, false);

    return 0;
}