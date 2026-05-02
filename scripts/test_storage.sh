#!/usr/bin/env bash
set -euo pipefail

echo "== storage-node standalone test =="

curl -s -X PUT localhost:9001/block/test-block --data-binary "hello-storage" > /dev/null

OUT=$(curl -s localhost:9001/block/test-block)

if [ "$OUT" != "hello-storage" ]; then
  echo "FAIL: expected hello-storage, got $OUT"
  exit 1
fi

echo "storage OK"
