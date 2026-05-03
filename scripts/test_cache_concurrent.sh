#!/usr/bin/env bash
set -euo pipefail

CACHE_URL="${CACHE_URL:-http://localhost:9100}"

echo "== cache concurrent test =="

curl -fsS "${CACHE_URL}/health" > /dev/null
curl -fsS -X POST "${CACHE_URL}/reset-stats" > /dev/null

PREFIX="concurrent-$(date +%s%N)"
CLIENTS=20

for i in $(seq 1 "$CLIENTS"); do
(
    KEY="${PREFIX}-${i}"
    VALUE="value-${i}"

    curl -fsS -X PUT "${CACHE_URL}/cache/${KEY}" \
      --data-binary "${VALUE}" > /dev/null

    READ="$(curl -fsS "${CACHE_URL}/cache/${KEY}")"

    if [ "${READ}" != "${VALUE}" ]; then
        echo "FAIL: key=${KEY}, expected=${VALUE}, got=${READ}"
        exit 1
    fi
) &
done

wait

STATS="$(curl -fsS "${CACHE_URL}/stats")"
echo "$STATS"

echo "$STATS" | grep -q "\"puts\":${CLIENTS}"
echo "$STATS" | grep -q "\"gets\":${CLIENTS}"
echo "$STATS" | grep -q "\"hits\":${CLIENTS}"

echo "cache concurrent OK"