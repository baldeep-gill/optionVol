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
#include "VisHandle.h"

std::string epoch_to_timestamp(long long epoch) {
    epoch -= 5 * 3600 * 1000;
    std::time_t t = static_cast<std::time_t>(epoch / 1000);
    std::tm tm = *std::localtime(&t);

    char buffer[30];
    std::strftime(buffer, sizeof(buffer), "%H:%M", &tm);
    return std::string(buffer);
}

// std::pair<double, double> linear_regression(const std::vector<double>& y) {
//     if (y.size() < 2) return {0.0, 0.0};
//     std::vector<double> x(y.size());
//     std::iota(x.begin(), x.end(), 1);

//     double n = static_cast<double>(y.size());
//     double sum_x = std::accumulate(x.begin(), x.end(), 0.0);
//     double sum_y = std::accumulate(y.begin(), y.end(), 0.0);
//     double sum_xy = 0.0, sum_x2 = 0.0;

//     for (size_t i = 0; i < x.size(); ++i) {
//         sum_xy += x[i] * y[i];
//         sum_x2 += x[i] * x[i];
//     }

//     double m = (n * sum_xy - sum_x * sum_y) / (n * sum_x2 - sum_x * sum_x);
//     double b = (sum_y - m * sum_x) / n;

//     return {m, b};
// }

int main() {
    const size_t thread_count = 30;
    ThreadPool pool(thread_count);

    std::string underlying = "SPX";
    std::string date = "2025-10-22";
    float strike = get_open_price(date);
    float range = 0.15;

    DataAggregates aggs = calculate_aggregates(underlying, strike, range, date, pool);

    VisHandle visHandle(aggs);
    visHandle.drawOverall(underlying, date);

    /*
    std::vector<double> x(aggs.timestamps.size());
    std::iota(x.begin(), x.end(), 1);

    auto f = matplot::figure(true);
    f->title("SPX " + date);
    matplot::hold(matplot::on);
    auto l_call = matplot::plot(x, aggs.calls, "-g")->line_width(2);
    auto l_put = matplot::plot(x, aggs.puts, "-r")->line_width(2);
    auto l_spot = matplot::plot(x, aggs.spot)->line_width(1.5);

    auto [m_call, b_call] = linear_regression(aggs.calls);
    auto [m_put, b_put] = linear_regression(aggs.puts);

    std::vector<double> y_call;
    y_call.reserve(x.size());
    std::vector<double> y_put;
    y_put.reserve(x.size());

    for (double xi: x) {
        y_call.push_back(m_call * xi + b_call);
        y_put.push_back(m_put * xi + b_put);
    }

    auto call_regression = matplot::plot(x, y_call, "--")->line_width(1).color("green");
    auto put_regression = matplot::plot(x, y_put, "--")->line_width(1).color("red");

    f->draw();
    */

    /* std::filesystem::create_directory("frames");
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
    } */

    // matplot::xticks(matplot::iota(1, 6, 78));
    // matplot::xticklabels({"09:35","10:05","10:35","11:05","11:35","12:05","12:35","13:05","13:35","14:05","14:35","15:05","15:35"});
    // matplot::show();

    return 0;
}