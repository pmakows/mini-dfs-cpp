#!/usr/bin/env bash
set -e

CACHE_URL="${CACHE_URL:-http://localhost:9100}"

echo "== Reset stats =="
curl -s "$CACHE_URL/reset-stats" > /dev/null

echo "== Basic test =="

echo "PUT test"
curl -s -X PUT "$CACHE_URL/cache/test" --data-binary "hello" > /dev/null

echo "GET hit"
curl -s "$CACHE_URL/cache/test" > /dev/null

echo "GET miss"
curl -s "$CACHE_URL/cache/missing" > /dev/null

echo
echo "== Stats =="
curl -s "$CACHE_URL/stats"
echo

echo "== HOT workload =="
curl -s "$CACHE_URL/reset-stats" > /dev/null

for i in {1..5}; do
  curl -s -X PUT "$CACHE_URL/cache/key$i" --data-binary "data$i" > /dev/null
done

for i in {1..200}; do
  curl -s "$CACHE_URL/cache/key1" > /dev/null
done

echo "HOT stats:"
curl -s "$CACHE_URL/stats"
echo

echo "== RANDOM workload =="
curl -s "$CACHE_URL/reset-stats" > /dev/null

for i in {1..200}; do
  curl -s "$CACHE_URL/cache/random$i" > /dev/null
done

echo "RANDOM stats:"
curl -s "$CACHE_URL/stats"
echo

echo "== MIXED workload =="
curl -s "$CACHE_URL/reset-stats" > /dev/null

for i in {1..5}; do
  curl -s -X PUT "$CACHE_URL/cache/key$i" --data-binary "data$i" > /dev/null
done

for i in {1..200}; do
  if (( i % 2 == 0 )); then
    curl -s "$CACHE_URL/cache/key1" > /dev/null
  else
    curl -s "$CACHE_URL/cache/random$i" > /dev/null
  fi
done

echo "MIXED stats:"
curl -s "$CACHE_URL/stats"
echo

echo "== DONE =="