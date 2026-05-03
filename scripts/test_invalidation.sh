#!/usr/bin/env bash
set -euo pipefail

DFS="./build/dfs"

echo "== cache invalidation test =="

echo "v1 data" > input.txt
$DFS put input.txt /inv.txt > /dev/null
$DFS get /inv.txt out1.txt > /dev/null

echo "v2 data" > input.txt
$DFS put input.txt /inv.txt > put.log
$DFS get /inv.txt out2.txt > get.log

diff input.txt out2.txt

grep -q "CACHE INVALIDATE" put.log
grep -q "CACHE HIT" get.log

rm -f input.txt out1.txt out2.txt put.log get.log

echo "INVALIDATION OK"