#!/usr/bin/env bash
set -euo pipefail

echo "== small file test (CACHE HIT expected) =="

KEY="small-file-test"
CACHE_URL="http://localhost:9100/cache/$KEY"

echo "[SMALL FILE] cleanup old key"
curl -fsS -X DELETE "$CACHE_URL" > /dev/null || true

echo "[SMALL FILE] PUT key=$KEY value=hello-small"
curl -fsS -X PUT "$CACHE_URL" --data-binary "hello-small" > /dev/null

echo "[SMALL FILE] GET #1"
OUT1=$(curl -fsS "$CACHE_URL")
echo "[SMALL FILE] OUT1=$OUT1"

echo "[SMALL FILE] GET #2"
OUT2=$(curl -fsS "$CACHE_URL")
echo "[SMALL FILE] OUT2=$OUT2"

echo "$OUT1" | grep -q "hello-small"
echo "$OUT2" | grep -q "hello-small"

echo "[SMALL FILE] verifying CACHE HIT in cache-node logs"
docker compose logs cache-node | grep "\[CACHE HIT\] key=$KEY"

echo "[SMALL FILE] CACHE HIT verified for key=$KEY"

curl -fsS -X DELETE "$CACHE_URL" > /dev/null

echo "small file OK"
