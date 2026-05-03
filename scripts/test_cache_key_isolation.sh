#!/usr/bin/env bash
set -euo pipefail

CACHE_URL="${CACHE_URL:-http://localhost:9100}"

echo "== cache key isolation test =="

curl -s -X DELETE "$CACHE_URL/cache/k1" > /dev/null || true
curl -s -X DELETE "$CACHE_URL/cache/k2" > /dev/null || true

curl -s -X PUT "$CACHE_URL/cache/k1" --data-binary "A" > /dev/null
curl -s -X PUT "$CACHE_URL/cache/k2" --data-binary "B" > /dev/null

OUT1=$(curl -s "$CACHE_URL/cache/k1")
OUT2=$(curl -s "$CACHE_URL/cache/k2")

echo "$OUT1" | grep -q "A"
echo "$OUT2" | grep -q "B"

curl -s -X DELETE "$CACHE_URL/cache/k1" > /dev/null || true
curl -s -X DELETE "$CACHE_URL/cache/k2" > /dev/null || true

echo "cache key isolation OK"
