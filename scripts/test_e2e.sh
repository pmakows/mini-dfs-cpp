#!/usr/bin/env bash
set -euo pipefail

DFS="./build/dfs"

echo "== E2E test (small file + cache) =="

echo "hello e2e" > input.txt

$DFS put input.txt /e2e.txt > /dev/null

$DFS get /e2e.txt out1.txt > get1.log
$DFS get /e2e.txt out2.txt > get2.log

diff input.txt out1.txt
diff input.txt out2.txt

grep -q "CACHE MISS" get1.log
grep -q "CACHE HIT" get2.log

echo "E2E OK"
