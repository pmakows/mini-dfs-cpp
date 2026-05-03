#!/usr/bin/env bash
set -euo pipefail

echo "== large binary file test =="

DFS="./build/dfs"

dd if=/dev/urandom of=big.bin bs=64K count=3 status=none

$DFS put big.bin big.bin
$DFS get big.bin big_out.bin

cmp big.bin big_out.bin

rm -f big.bin big_out.bin

echo "large file OK"