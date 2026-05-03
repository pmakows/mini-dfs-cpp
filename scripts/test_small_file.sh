#!/usr/bin/env bash
set -euo pipefail

echo "== small file test (CACHE HIT expected) =="

KEY="small-file-test"
CACHE_URL="http://localhost:9100/cache/$KEY"
STATS_URL="http://localhost:9100/stats"
RESET_STATS_URL="http://localhost:9100/reset-stats"

echo "[SMALL FILE] cleanup old key"
curl -fss -X DELETE "$CACHE_URL" > /dev/null || true

echo "[SMALL FILE] reset stats"
curl -s "$RESET_STATS_URL" > /dev/null || true

echo "[SMALL FILE] PUT key=$KEY value=hello-small"
curl -fss -X PUT "$CACHE_URL" --data-binary "hello-small" > /dev/null

echo "[SMALL FILE] GET #1"
OUT1=$(curl -fss "$CACHE_URL")
echo "[SMALL FILE] OUT1=$OUT1"

echo "[SMALL FILE] GET #2"
OUT2=$(curl -fss "$CACHE_URL")
echo "[SMALL FILE] OUT2=$OUT2"

echo "$OUT1" | grep -q "hello-small"
echo "$OUT2" | grep -q "hello-small"

echo "[SMALL FILE] verifying CACHE HIT using stats API"
STATS=$(curl -s "$STATS_URL")
echo "[SMALL FILE] stats=$STATS"

HITS=$(echo "$STATS" | grep -o '"hits":[0-9]*' | cut -d ':' -f2)

if [ "$HITS" -lt 2 ]; then
  echo "FAIL: expected at least 2 cache hits, got $HITS"
  exit 1
fi

curl -fss -X DELETE "$CACHE_URL" > /dev/null || true

echo "small file OK"