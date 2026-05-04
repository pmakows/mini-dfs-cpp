#!/usr/bin/env bash
set -euo pipefail

DFS="./build/dfs"

echo "== cache invalidation test =="

echo "v1 data" > input.txt
$DFS put input.txt /inv.txt > /dev/null

# First read should populate cache with v1
$DFS get /inv.txt out1.txt > /dev/null

echo "v2 data" > input.txt

# Updating the same path should invalidate old cached value
$DFS put input.txt /inv.txt > put.log

# First read after invalidation should fetch fresh data and repopulate cache
$DFS get /inv.txt out2.txt > get1.log

# Second read should come from cache
$DFS get /inv.txt out3.txt > get2.log

# Both reads must return the updated content, not stale v1 data
diff input.txt out2.txt
diff input.txt out3.txt

# PUT must invalidate cache entry
grep -q "CACHE INVALIDATE" put.log

# After invalidation, first GET should miss, second GET should hit
grep -q "CACHE MISS" get1.log
grep -q "CACHE HIT" get2.log

rm -f input.txt out1.txt out2.txt out3.txt put.log get1.log get2.log

echo "INVALIDATION OK"