#include "VisHandle.h"

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

void VisHandle::drawChartOnce() {
    std::vector<double> x(VisHandle::aggregates.timestamps.size());
    std::iota(x.begin(), x.end(), 1);

    matplot::hold(matplot::on);
    auto l_call = matplot::plot(x, VisHandle::aggregates.calls, "-g")->line_width(2);
    auto l_put = matplot::plot(x, VisHandle::aggregates.puts, "-r")->line_width(2);
    auto l_spot = matplot::plot(x, VisHandle::aggregates.spot)->line_width(1.5);

    auto [m_call, b_call] = linear_regression(VisHandle::aggregates.calls);
    auto [m_put, b_put] = linear_regression(VisHandle::aggregates.puts);

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

    VisHandle::figure->draw();
}

void VisHandle::drawOverall(std::string underlying, std::string date) {
    VisHandle::figure->title(underlying + " " + date);
    VisHandle::drawChartOnce();

    matplot::show();
}
