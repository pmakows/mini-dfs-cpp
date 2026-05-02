#!/usr/bin/env bash
set -euo pipefail

DFS="./build/dfs"

echo "== client split test =="

dd if=/dev/urandom of=split_test.bin bs=64K count=3 status=none

OUT=$($DFS split split_test.bin)

echo "$OUT" | grep -q "Chunks: 3"
echo "$OUT" | grep -q "Chunk 0 size=65536"
echo "$OUT" | grep -q "Chunk 1 size=65536"
echo "$OUT" | grep -q "Chunk 2 size=65536"

echo "split OK"
