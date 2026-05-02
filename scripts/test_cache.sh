#!/usr/bin/env bash
set -e

echo "== cache standalone test =="

curl -s -X PUT localhost:9100/cache/test --data-binary "hello" > /dev/null

OUT=$(curl -s localhost:9100/cache/test)
if [ "$OUT" != "hello" ]; then
  echo "FAIL: expected hello, got $OUT"
  exit 1
fi

curl -s -X DELETE localhost:9100/cache/test > /dev/null

CODE=$(curl -s -o /dev/null -w "%{http_code}" localhost:9100/cache/test)
if [ "$CODE" != "404" ]; then
  echo "FAIL: expected 404 after delete"
  exit 1
fi

echo "cache OK"
