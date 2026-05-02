#!/usr/bin/env bash
set -euo pipefail

echo "== metadata-service standalone test =="

curl -s -X POST localhost:9000/files \
  -H "Content-Type: application/json" \
  -d '{
    "path": "/meta-test.txt",
    "blocks": [
      {
        "id": "block-meta-test",
        "nodes": ["localhost:9001"],
        "size": 12
      }
    ]
  }' > /dev/null

OUT=$(curl -s localhost:9000/files/meta-test.txt)

echo "$OUT" | grep -q "block-meta-test"
echo "$OUT" | grep -q "localhost:9001"

echo "metadata OK"
