#pragma once
#include "contracts.h"
#include <matplot/matplot.h>

class VisHandle {
    public:
        VisHandle(DataAggregates aggs) : aggregates(aggs), figure(matplot::figure(true)) {};
        void updateData(DataAggregates aggs);
        void drawOverall(std::string underlying, std::string date);

    private:
        DataAggregates aggregates;
        matplot::figure_handle figure;
        void drawChartOnce();
        std::pair<double, double> linear_regression(const std::vector<double>& y);
};