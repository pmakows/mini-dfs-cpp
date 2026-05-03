#!/usr/bin/env bash
set -euo pipefail

echo "== storage-node standalone test =="

STORAGE_BIN="./build/storage_node"

if [[ ! -x "$STORAGE_BIN" ]]; then
  echo "ERROR: storage_node binary not found"
  find build -type f -executable || true
  exit 1
fi

TMP_DIR="$(mktemp -d)"

cleanup() {
  kill "$PID" 2>/dev/null || true
  rm -rf "$TMP_DIR"
}
trap cleanup EXIT

STORAGE_DATA_DIR="$TMP_DIR/blocks" "$STORAGE_BIN" &
PID=$!

sleep 1

curl -fsS http://localhost:8080/health >/dev/null

echo "hello storage" | curl -fsS -X PUT http://localhost:8080/block/test-block --data-binary @- >/dev/null

OUT="$(curl -fsS http://localhost:8080/block/test-block)"

if [[ "$OUT" != "hello storage" ]]; then
  echo "ERROR: storage returned unexpected data"
  exit 1
fi

echo "storage OK"