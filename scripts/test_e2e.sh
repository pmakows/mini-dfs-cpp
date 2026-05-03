#!/usr/bin/env bash
set -euo pipefail

echo "== E2E test (small file + cache) =="

DFS_BIN="./build/client/dfs"

if [[ ! -x "$DFS_BIN" ]]; then
  DFS_BIN="./build/dfs"
fi

if [[ ! -x "$DFS_BIN" ]]; then
  echo "ERROR: dfs binary not found"
  find build -type f -executable || true
  exit 1
fi

echo "hello dfs e2e" > e2e_test.txt

"$DFS_BIN" put e2e_test.txt e2e_test.txt
"$DFS_BIN" get e2e_test.txt e2e_out.txt

cmp e2e_test.txt e2e_out.txt

curl -fsS http://localhost:9100/cache/e2e_test.txt >/dev/null

rm -f e2e_test.txt e2e_out.txt

echo "E2E OK"