#!/usr/bin/env bash
set -euo pipefail

CACHE_URL="${CACHE_URL:-http://localhost:9100}"

echo "== analyzer test =="

curl -fsS "${CACHE_URL}/health" > /dev/null

# NO_DATA
curl -fsS -X POST "${CACHE_URL}/reset-stats" > /dev/null

OUT="$(curl -fsS "${CACHE_URL}/analyze")"
echo "NO_DATA -> ${OUT}"

echo "${OUT}" | grep -q "NO_DATA" || {
    echo "FAIL: expected NO_DATA"
    exit 1
}

# BAD: only misses
curl -fsS -X POST "${CACHE_URL}/reset-stats" > /dev/null

curl -s "${CACHE_URL}/cache/missing-a" > /dev/null || true
curl -s "${CACHE_URL}/cache/missing-b" > /dev/null || true
curl -s "${CACHE_URL}/cache/missing-c" > /dev/null || true

OUT="$(curl -fsS "${CACHE_URL}/analyze")"
echo "BAD -> ${OUT}"

echo "${OUT}" | grep -q "BAD" || {
    echo "FAIL: expected BAD"
    exit 1
}

# WARN: one hit, one miss => hit_rate 0.5
curl -fsS -X POST "${CACHE_URL}/reset-stats" > /dev/null

curl -fsS -X PUT "${CACHE_URL}/cache/warn-key" --data-binary "x" > /dev/null
curl -fsS "${CACHE_URL}/cache/warn-key" > /dev/null
curl -s "${CACHE_URL}/cache/warn-missing" > /dev/null || true

OUT="$(curl -fsS "${CACHE_URL}/analyze")"
echo "WARN -> ${OUT}"

echo "${OUT}" | grep -q "WARN" || {
    echo "FAIL: expected WARN"
    exit 1
}

# OK: only hits
curl -fsS -X POST "${CACHE_URL}/reset-stats" > /dev/null

curl -fsS -X PUT "${CACHE_URL}/cache/ok-key" --data-binary "x" > /dev/null
curl -fsS "${CACHE_URL}/cache/ok-key" > /dev/null
curl -fsS "${CACHE_URL}/cache/ok-key" > /dev/null

OUT="$(curl -fsS "${CACHE_URL}/analyze")"
echo "OK -> ${OUT}"

echo "${OUT}" | grep -q "OK" || {
    echo "FAIL: expected OK"
    exit 1
}

echo "analyzer test OK"