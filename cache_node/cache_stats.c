#include "cache_stats.h"

#include <stdio.h>
#include <string.h>

void cache_stats_init(cache_stats_t *stats) {
    memset(stats, 0, sizeof(cache_stats_t));
}

void cache_stats_record_hit(cache_stats_t *stats) {
    stats->gets++;
    stats->hits++;
}

void cache_stats_record_miss(cache_stats_t *stats) {
    stats->gets++;
    stats->misses++;
}

void cache_stats_record_put(cache_stats_t *stats) {
    stats->puts++;
}

void cache_stats_record_invalidation(cache_stats_t *stats) {
    stats->invalidations++;
}

double cache_stats_hit_rate(const cache_stats_t *stats) {
    if (stats->gets == 0) {
        return 0.0;
    }

    return (double)stats->hits / (double)stats->gets;
}

void cache_stats_to_json(
    const cache_stats_t *stats,
    char *buf,
    size_t size
) {
    snprintf(
        buf,
        size,
        "{"
        "\"hits\":%lu,"
        "\"misses\":%lu,"
        "\"gets\":%lu,"
        "\"puts\":%lu,"
        "\"invalidations\":%lu,"
        "\"hit_rate\":%.3f"
        "}",
        stats->hits,
        stats->misses,
        stats->gets,
        stats->puts,
        stats->invalidations,
        cache_stats_hit_rate(stats)
    );
}