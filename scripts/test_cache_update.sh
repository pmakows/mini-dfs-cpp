#!/usr/bin/env bash
set -euo pipefail

CACHE_URL="${CACHE_URL:-http://localhost:9100}"
URL="$CACHE_URL/cache/update-test"

echo "== cache update test =="

curl -s -X DELETE "$URL" > /dev/null || true

curl -s -X PUT "$URL" --data-binary "v1" > /dev/null
curl -s -X PUT "$URL" --data-binary "v2" > /dev/null

OUT=$(curl -s "$URL")

echo "$OUT" | grep -q "v2"

curl -s -X DELETE "$URL" > /dev/null || true

echo "cache update OK"
