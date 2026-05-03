#ifndef CACHE_STATS_H
#define CACHE_STATS_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    unsigned long hits;
    unsigned long misses;
    unsigned long gets;
    unsigned long puts;
    unsigned long invalidations;
} cache_stats_t;

void cache_stats_init(cache_stats_t *stats);

void cache_stats_record_hit(cache_stats_t *stats);
void cache_stats_record_miss(cache_stats_t *stats);
void cache_stats_record_put(cache_stats_t *stats);
void cache_stats_record_invalidation(cache_stats_t *stats);

double cache_stats_hit_rate(const cache_stats_t *stats);

void cache_stats_to_json(
    const cache_stats_t *stats,
    char *buf,
    size_t size
);

#ifdef __cplusplus
}
#endif

#endif