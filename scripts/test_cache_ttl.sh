#!/usr/bin/env bash
set -euo pipefail

CACHE_URL="${CACHE_URL:-http://localhost:9100}"
KEY="ttl-test"
URL="$CACHE_URL/cache/$KEY"

echo "== cache TTL test =="

# cleanup
curl -s -X DELETE "$URL" > /dev/null || true

# put
curl -s -X PUT "$URL" --data-binary "hello-ttl" > /dev/null

# immediate read (should HIT)
OUT=$(curl -s "$URL")
echo "$OUT" | grep -q "hello-ttl"

echo "[TTL] waiting for expiration (35s)..."
sleep 35

# after TTL (should MISS → 404)
STATUS=$(curl -s -o /dev/null -w "%{http_code}" "$URL")

if [ "$STATUS" -eq 200 ]; then
  echo "FAIL: expected cache miss after TTL, got HIT"
  exit 1
fi

echo "TTL test OK"