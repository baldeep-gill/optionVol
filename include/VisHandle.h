#pragma once
#include "contracts.h"
#include <matplot/matplot.h>

class VisHandle {
    public:
        VisHandle(DataAggregates aggs) : aggregates(aggs), figure(matplot::figure(true)) {};
        void updateData(DataAggregates aggs);
        void drawOverall(std::string underlying, std::string date);
        void drawStepped(std::string underlying, std::string date, size_t interval);

    private:
        DataAggregates aggregates;
        matplot::figure_handle figure;
        matplot::line_handle l_call, l_put, l_spot, call_regression, put_regression;
        void drawChartOnce(std::vector<long long> timestamps, std::vector<double> calls, std::vector<double> puts, std::vector<double> spot, std::vector<double> fit_call, std::vector<double> fit_put);
        std::pair<double, double> linear_regression(const std::vector<double>& y);
};