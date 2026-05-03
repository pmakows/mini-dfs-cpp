#ifndef CACHE_H
#define CACHE_H

#include "cache_stats.h"

#include <stddef.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CACHE_SIZE 128
#define CACHE_KEY_SIZE 256
#define CACHE_VALUE_SIZE 8192
#define CACHE_TTL_SECONDS 30

typedef struct {
    int valid;
    char key[CACHE_KEY_SIZE];
    char value[CACHE_VALUE_SIZE];
    size_t value_len;
    time_t timestamp;
} cache_entry_t;

typedef struct {
    cache_entry_t entries[CACHE_SIZE];
    cache_stats_t stats;
} cache_t;

void cache_init(cache_t *cache);

int cache_get(
    cache_t *cache,
    const char *key,
    char *out,
    size_t out_size,
    size_t *out_len
);

void cache_put(
    cache_t *cache,
    const char *key,
    const char *value,
    size_t value_len
);

void cache_invalidate(cache_t *cache, const char *key);

void cache_reset_stats(cache_t *cache);

#ifdef __cplusplus
}
#endif

#endif