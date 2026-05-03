#include "cache_analyzer.h"

cache_analyzer_report_t cache_analyze(const cache_stats_t* stats) {
    cache_analyzer_report_t report;

    if (stats == NULL || stats->gets == 0) {
        report.status = "NO_DATA";
        report.message = "No traffic observed";
        report.hit_rate = 0.0;
        return report;
    }

    double hit_rate = (double)stats->hits / (double)stats->gets;

    report.hit_rate = hit_rate;

    if (hit_rate >= 0.8) {
        report.status = "OK";
        report.message = "Cache is healthy";
    } else if (hit_rate >= 0.5) {
        report.status = "WARN";
        report.message = "Cache hit rate is moderate";
    } else {
        report.status = "BAD";
        report.message = "Cache hit rate is low";
    }

    return report;
}