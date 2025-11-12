#include "VisHandle.h"
#include <chrono>

std::pair<double, double> VisHandle::linear_regression(const std::vector<double>& y) {
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

void VisHandle::updateData(DataAggregates aggs) {
    VisHandle aggregates = aggs;
}

void VisHandle::drawChartOnce(std::vector<long long> timestamps, std::vector<double> calls, std::vector<double> puts, std::vector<double> spot, std::vector<double> fit_call, std::vector<double> fit_put) {
    
    VisHandle::l_call->y_data(calls).line_width(2);

    VisHandle::l_put->y_data(puts).line_width(2);

    VisHandle::l_spot->y_data(spot).line_width(1.5);

    VisHandle::call_regression->y_data(fit_call).line_width(1).color("green");
    VisHandle::put_regression->y_data(fit_put).line_width(1).color("red");

    VisHandle::figure->draw();
}

void VisHandle::drawOverall(std::string underlying, std::string date) {
    size_t size = VisHandle::aggregates.timestamps.size();
    std::vector<double> x(size);
    std::iota(x.begin(), x.end(), 1);

    matplot::hold(matplot::on);
    VisHandle::figure->title(underlying + " " + date);

    auto [m_call, b_call] = linear_regression(VisHandle::aggregates.calls);
    auto [m_put, b_put] = linear_regression(VisHandle::aggregates.puts);

    std::vector<double> y_call;
    y_call.reserve(size);
    std::vector<double> y_put;
    y_put.reserve(size);

    for (double xi: x) {
        y_call.push_back(m_call * xi + b_call);
        y_put.push_back(m_put * xi + b_put);
    }

    VisHandle::l_call = matplot::plot(x, VisHandle::aggregates.calls, "-g");
    VisHandle::l_put = matplot::plot(x, VisHandle::aggregates.puts, "-r");
    VisHandle::l_spot = matplot::plot(x, VisHandle::aggregates.spot);

    VisHandle::call_regression = matplot::plot(x, y_call, "--");
    VisHandle::put_regression = matplot::plot(x, y_put, "--");

    VisHandle::drawChartOnce(VisHandle::aggregates.timestamps, VisHandle::aggregates.calls, VisHandle::aggregates.puts, VisHandle::aggregates.spot, y_call, y_put);

    matplot::show();
}

void VisHandle::drawStepped(std::string underlying, std::string date, size_t interval) {
    size_t size = VisHandle::aggregates.timestamps.size();
    std::vector<long long> timestamps;
    std::vector<double> calls, puts, spot;

    std::vector<double> x(size);
    std::iota(x.begin(), x.end(), 1);

    matplot::hold(matplot::on);

    VisHandle::l_call = matplot::plot(x, calls, "-g");
    VisHandle::l_put = matplot::plot(x, puts, "-r");
    VisHandle::l_spot = matplot::plot(x, spot);

    VisHandle::call_regression = matplot::plot(x, calls, "--");
    VisHandle::put_regression = matplot::plot(x, puts, "--");

    for (size_t i = 0; i < size; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(interval));

        timestamps.push_back(VisHandle::aggregates.timestamps[i]);
        calls.push_back(VisHandle::aggregates.calls[i]);
        puts.push_back(VisHandle::aggregates.puts[i]);
        spot.push_back(VisHandle::aggregates.spot[i]);


        auto [m_call, b_call] = linear_regression(calls);
        auto [m_put, b_put] = linear_regression(puts);

        std::vector<double> y_call;
        y_call.reserve(x.size());
        std::vector<double> y_put;
        y_put.reserve(x.size());

        for (double xi: x) {
            y_call.push_back(m_call * xi + b_call);
            y_put.push_back(m_put * xi + b_put);
        }

        VisHandle::drawChartOnce(timestamps, calls, puts, spot, y_call, y_put);
    }

    matplot::show();
}