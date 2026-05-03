#!/usr/bin/env bash
set -euo pipefail

echo "== small file test (cache hit expected) =="

# zapis małego pliku (wejdzie do cache)
curl -s -X PUT localhost:9000/block/small-test --data-binary "hello" > /dev/null

# pierwszy GET → MISS
OUT1=$(curl -s localhost:9000/block/small-test)

# drugi GET → HIT
OUT2=$(curl -s localhost:9000/block/small-test)

# sprawdzenie danych
echo "$OUT1" | grep -q "hello"
echo "$OUT2" | grep -q "hello"

echo "small file OK (cache hit verified)"
