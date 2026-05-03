#!/usr/bin/env bash
set -euo pipefail

echo "== client split/chunking test =="

DFS_BIN="./build/client/dfs"

if [[ ! -x "$DFS_BIN" ]]; then
  DFS_BIN="./build/dfs"
fi

if [[ ! -x "$DFS_BIN" ]]; then
  echo "ERROR: dfs binary not found"
  echo "Available executables in build/:"
  find build -type f -executable || true
  exit 1
fi

dd if=/dev/urandom of=split_test.bin bs=64K count=3 status=none

"$DFS_BIN" put split_test.bin split_test.bin
"$DFS_BIN" get split_test.bin split_test_out.bin

cmp split_test.bin split_test_out.bin

rm -f split_test.bin split_test_out.bin

echo "split/chunking OK"