#include <iostream>
#include "contracts.h"
#include "HttpUtils.h"
#include "thread_pool.h"
#include <cstdlib>
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <filesystem>
#include <thread>
#include <ctime>
#include <matplot/matplot.h>

std::string epoch_to_timestamp(long long epoch) {
    epoch -= 5 * 3600 * 1000;
    std::time_t t = static_cast<std::time_t>(epoch / 1000);
    std::tm tm = *std::localtime(&t);

    char buffer[30];
    std::strftime(buffer, sizeof(buffer), "%H:%M", &tm);
    return std::string(buffer);
}

std::pair<double, double> linear_regression(const std::vector<double>& y) {
    if (y.size() < 2) return {0.0, 0.0};
    std::vector<double> x(y.size());
    std::iota(x.begin(), x.end(), 1);

    double n = static_cast<double>(y.size());
    double sum_x = std::accumulate(x.begin(), x.end(), 0.0);
    double sum_y = std::accumulate(y.begin(), y.end(), 0.0);
    double sum_xy = 0.0, sum_x2 = 0.0;

    for (size_t i = 0; i < x.size(); ++i) {
        sum_xy += x[i] * y[i];
        sum_x2 += x[i] * x[i];
    }

    double m = (n * sum_xy - sum_x * sum_y) / (n * sum_x2 - sum_x * sum_x);
    double b = (sum_y - m * sum_x) / n;

    return {m, b};
}

int main() {
    const size_t thread_count = 30;
    ThreadPool pool(thread_count);

    std::string underlying = "SPX";
    std::string date = "2025-10-22";
    float strike = get_open_price(date);
    float range = 0.15;

    // std::cout << "Enter date (YYYY-MM-DD): ";
    // std::cin >> date;
    // std::cin.clear();
    // std::cin.ignore(10000, '\n');

    std::vector<Contract> call_contracts = get_contracts(underlying, strike, range, "call", date);
    std::vector<Contract> put_contracts = get_contracts(underlying, strike, range, "put", date);
        
    auto start = std::chrono::high_resolution_clock::now();

    std::vector<ContractVolumes> call_volumes = get_volume_par(pool, call_contracts, date);
    std::vector<ContractVolumes> put_volumes = get_volume_par(pool, put_contracts, date);
    std::cout << "Fetched volumes for " << call_volumes.size() + put_volumes.size() << " contracts.\n";

    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
    std::cout << "Took " << duration.count() << " milliseconds.\n";

    std::map<long long, float> call_acc;         // For each timestamp store strike_t * vol_s_t
    std::map<long long, size_t> call_vol_aggs;   // For each timestamp store vol_t

    std::map<long long, float> put_acc;
    std::map<long long, size_t> put_vol_aggs;

    for (ContractVolumes& vec: call_volumes) {
        for (VolumePoint& item: vec.slices) {
            call_acc[item.timestamp] += vec.strike * item.volume;
            call_vol_aggs[item.timestamp] += item.volume;
        }
    }

    for (ContractVolumes& vec: put_volumes) {
        for (VolumePoint& item: vec.slices) {
            put_acc[item.timestamp] += vec.strike * item.volume;
            put_vol_aggs[item.timestamp] += item.volume;
        }
    }

    std::map<long long, float> spx_price = get_price(date);

    // std::cout << "CALLS:\n";
    // for (auto& entry: call_acc) {
    //     float temp = entry.second / call_vol_aggs[entry.first];
    //     entry.second = temp;
    //     std::cout << epoch_to_timestamp(entry.first) << ": " << temp << "\n";
    // }

    // std::cout << "PUTS:\n";
    // for (auto& entry: put_acc) {
    //     float temp = entry.second / put_vol_aggs[entry.first];
    //     entry.second = temp;
    //     std::cout << epoch_to_timestamp(entry.first) << ": " << temp << "\n";
    // }

    std::vector<double> call_vwas, put_vwas, spot;
    std::vector<double> x(78);
    std::iota(x.begin(), x.end(), 1);

    auto f = matplot::figure(true);
    f->title("SPX " + date);
    matplot::hold(matplot::on);
    auto l_call = matplot::plot(x, call_vwas, "-g");
    auto l_put = matplot::plot(x, put_vwas, "-r");
    auto l_spot = matplot::plot(x, spot);

    auto call_regression = matplot::plot(x, call_vwas, "--");
    auto put_regression = matplot::plot(x, put_vwas, "--");


    std::filesystem::create_directory("frames");
    size_t count = 0;
    for (auto& [ts, acc]: call_acc) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        if (call_vol_aggs[ts] > 0) {
            call_vwas.push_back(acc / call_vol_aggs[ts]);
            put_vwas.push_back(put_acc.count(ts) && put_vol_aggs[ts] > 0 ? put_acc[ts] / put_vol_aggs[ts] : NAN);
            spot.push_back(spx_price.count(ts) ? spx_price[ts] : NAN);
        }

        l_call->y_data(call_vwas).line_width(2);
        l_put->y_data(put_vwas).line_width(2);
        l_spot->y_data(spot).line_width(1.5);

        auto [m_call, b_call] = linear_regression(call_vwas);
        auto [m_put, b_put] = linear_regression(put_vwas);
        
        std::vector<double> y_call;
        y_call.reserve(x.size());
        std::vector<double> y_put;
        y_put.reserve(x.size());

        for (double xi: x) {
            y_call.push_back(m_call * xi + b_call);
            y_put.push_back(m_put * xi + b_put);
        }

        call_regression->y_data(y_call).x_data(x).color("green").line_width(1);
        put_regression->y_data(y_put).x_data(x).color("red").line_width(1);

        f->draw();

        std::ostringstream filename;
        filename << "frames/frame_" << std::setw(3) << std::setfill('0') << count << ".png";
        matplot::save(filename.str());
        count++;
    }

    // matplot::xticks(matplot::iota(1, 6, 78));
    // matplot::xticklabels({"09:35","10:05","10:35","11:05","11:35","12:05","12:35","13:05","13:35","14:05","14:35","15:05","15:35"});
    matplot::show();

    return 0;
}