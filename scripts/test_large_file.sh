#!/usr/bin/env bash
set -euo pipefail

DFS="./build/dfs"

echo "== large binary file test =="

dd if=/dev/urandom of=big.bin bs=64K count=3 status=none

$DFS put big.bin /big.bin | tee put_big.log
$DFS get /big.bin big_out.bin | tee get_big.log

diff big.bin big_out.bin

grep -q "Stored chunk 0" put_big.log
grep -q "Stored chunk 1" put_big.log
grep -q "Stored chunk 2" put_big.log
grep -q "CACHE SKIP" get_big.log

echo "large binary file OK"
