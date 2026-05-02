#!/usr/bin/env bash
set -euo pipefail

DFS="./build/dfs"

echo "== cache invalidation test =="

echo "v1 data" > input.txt
$DFS put input.txt /inv.txt > /dev/null

$DFS get /inv.txt out1.txt > /dev/null
$DFS get /inv.txt out2.txt > get_hit.log

echo "v2 data" > input.txt
$DFS put input.txt /inv.txt > put.log

$DFS get /inv.txt out3.txt > get_miss.log
$DFS get /inv.txt out4.txt > get_hit2.log

diff input.txt out3.txt
diff input.txt out4.txt

grep -q "CACHE INVALIDATE" put.log
grep -q "CACHE MISS" get_miss.log
grep -q "CACHE HIT" get_hit2.log

echo "INVALIDATION OK"
