#include "cache.h"

#include <stdio.h>
#include <string.h>

static unsigned long cache_hash(const char *key)
{
    unsigned long hash = 5381;
    int c;

    while ((c = *key++)) {
        hash = ((hash << 5) + hash) + c;
    }

    return hash;
}

void cache_init(cache_t *cache)
{
    memset(cache, 0, sizeof(*cache));
}

int cache_get(
    cache_t *cache,
    const char *key,
    char *out,
    size_t out_size,
    size_t *out_len
)
{
    unsigned long idx = cache_hash(key) % CACHE_SIZE;
    cache_entry_t *entry = &cache->entries[idx];

    if (!entry->valid) {
        return 0;
    }

    if (strcmp(entry->key, key) != 0) {
        return 0;
    }

    time_t now = time(NULL);

    if ((now - entry->timestamp) > CACHE_TTL_SECONDS) {
        entry->valid = 0;
        return 0;
    }

    if (entry->value_len > out_size) {
        return 0;
    }

    memcpy(out, entry->value, entry->value_len);
    *out_len = entry->value_len;

    return 1;
}

void cache_put(
    cache_t *cache,
    const char *key,
    const char *value,
    size_t value_len
)
{
    if (value_len > CACHE_VALUE_SIZE) {
        return;
    }

    unsigned long idx = cache_hash(key) % CACHE_SIZE;
    cache_entry_t *entry = &cache->entries[idx];

    memset(entry, 0, sizeof(*entry));

    entry->valid = 1;
    snprintf(entry->key, CACHE_KEY_SIZE, "%s", key);

    memcpy(entry->value, value, value_len);
    entry->value_len = value_len;
    entry->timestamp = time(NULL);
}

void cache_invalidate(cache_t *cache, const char *key)
{
    unsigned long idx = cache_hash(key) % CACHE_SIZE;
    cache_entry_t *entry = &cache->entries[idx];

    if (entry->valid && strcmp(entry->key, key) == 0) {
        entry->valid = 0;
    }
}