#pragma once
#include "contracts.h"
#include <matplot/matplot.h>

class VisHandle {
    public:
        VisHandle();
        void updateData(DataAggregates aggs);
        void drawOverall(std::string underlying, std::string date);
        void drawStepped(std::string underlying, std::string date, size_t interval, bool save_frames);

    private:
        DataAggregates aggregates;
        matplot::figure_handle figure;
        matplot::line_handle l_call, l_put, l_spot, call_regression, put_regression;
        void drawChartOnce(std::vector<long long>& timestamps, std::vector<double>& calls, std::vector<double>& puts, std::vector<double>& spot, std::vector<double>& fit_call, std::vector<double>& fit_put);
        std::pair<double, double> linear_regression(const std::vector<double>& y);
};

inline VisHandle::VisHandle() {
    VisHandle::figure = matplot::figure(true);

    VisHandle::figure->size(800, 800);

    matplot::xticks(matplot::iota(0, 6, 78));
    matplot::xticklabels({"09:30","10:00","10:30","11:00","11:30","12:00","12:30","13:00","13:30","14:00","14:30","15:00","15:30", "16:00"});
}