#!/usr/bin/env bash
set -euo pipefail

echo "== client split test =="

DFS_BIN="./build/client/dfs"

if [ ! -x "$DFS_BIN" ]; then
  DFS_BIN="./build/dfs"
fi

if [ ! -x "$DFS_BIN" ]; then
  echo "ERROR: dfs binary not found"
  echo "Available executables in build/:"
  find build -type f -executable || true
  exit 1
fi

dd if=/dev/urandom of=split_test.bin bs=64K count=3 status=none

OUT=$("$DFS_BIN" split split_test.bin)

echo "$OUT" | grep -q "Chunks: 3"
echo "$OUT" | grep -q "Chunk 0 size=65536"
echo "$OUT" | grep -q "Chunk 1 size=65536"
echo "$OUT" | grep -q "Chunk 2 size=65536"

rm -f split_test.bin

echo "split OK"
