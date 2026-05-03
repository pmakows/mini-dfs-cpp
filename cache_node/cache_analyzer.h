#ifndef CACHE_ANALYZER_H
#define CACHE_ANALYZER_H

#include "cache_stats.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const char* status;
    const char* message;
    double hit_rate;
} cache_analyzer_report_t;

cache_analyzer_report_t cache_analyze(const cache_stats_t* stats);

#ifdef __cplusplus
}
#endif

#endif