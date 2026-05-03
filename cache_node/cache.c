#include "cache.h"

#include <string.h>
#include <time.h>

void cache_init(cache_t *cache) {
    memset(cache, 0, sizeof(cache_t));
    cache_stats_init(&cache->stats);
}

static int cache_find_entry(cache_t *cache, const char *key) {
    time_t now = time(NULL);

    for (int i = 0; i < CACHE_SIZE; i++) {
        cache_entry_t *entry = &cache->entries[i];

        if (!entry->valid) {
            continue;
        }

        if ((now - entry->timestamp) > CACHE_TTL_SECONDS) {
            entry->valid = 0;
            continue;
        }

        if (strcmp(entry->key, key) == 0) {
            return i;
        }
    }

    return -1;
}

static int cache_find_free_slot(cache_t *cache) {
    time_t now = time(NULL);

    for (int i = 0; i < CACHE_SIZE; i++) {
        cache_entry_t *entry = &cache->entries[i];

        if (!entry->valid) {
            return i;
        }

        if ((now - entry->timestamp) > CACHE_TTL_SECONDS) {
            entry->valid = 0;
            return i;
        }
    }

    return 0;
}

int cache_get(
    cache_t *cache,
    const char *key,
    char *out,
    size_t out_size,
    size_t *out_len
) {
    int index = cache_find_entry(cache, key);

    if (index < 0) {
        cache_stats_record_miss(&cache->stats);
        return 0;
    }

    cache_entry_t *entry = &cache->entries[index];

    if (entry->value_len > out_size) {
        cache_stats_record_miss(&cache->stats);
        return 0;
    }

    memcpy(out, entry->value, entry->value_len);
    *out_len = entry->value_len;

    cache_stats_record_hit(&cache->stats);

    return 1;
}

void cache_put(
    cache_t *cache,
    const char *key,
    const char *value,
    size_t value_len
) {
    if (value_len > CACHE_VALUE_SIZE) {
        return;
    }

    int index = cache_find_entry(cache, key);

    if (index < 0) {
        index = cache_find_free_slot(cache);
    }

    cache_entry_t *entry = &cache->entries[index];

    memset(entry, 0, sizeof(cache_entry_t));

    entry->valid = 1;
    strncpy(entry->key, key, CACHE_KEY_SIZE - 1);
    entry->key[CACHE_KEY_SIZE - 1] = '\0';

    memcpy(entry->value, value, value_len);
    entry->value_len = value_len;
    entry->timestamp = time(NULL);

    cache_stats_record_put(&cache->stats);
}

void cache_invalidate(cache_t *cache, const char *key) {
    int index = cache_find_entry(cache, key);

    if (index >= 0) {
        memset(&cache->entries[index], 0, sizeof(cache_entry_t));
    }

    cache_stats_record_invalidation(&cache->stats);
}

void cache_reset_stats(cache_t *cache) {
    cache_stats_init(&cache->stats);
}