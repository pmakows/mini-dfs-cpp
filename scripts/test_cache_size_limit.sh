#!/usr/bin/env bash
set -euo pipefail

CACHE_URL="${CACHE_URL:-http://localhost:9100}"

echo "== cache size limit test =="

curl -fsS "${CACHE_URL}/health" > /dev/null

KEY="cache-size-limit-test-$(date +%s%N)"
SMALL_VALUE="small-cache-value"

TMP_BIG="$(mktemp)"
TMP_GET="$(mktemp)"

python3 - <<'PY' > "$TMP_BIG"
print("x" * 200000, end="")
PY

cleanup() {
    rm -f "$TMP_BIG" "$TMP_GET"
}
trap cleanup EXIT

curl -fsS -X PUT "${CACHE_URL}/cache/${KEY}-small" \
  --data-binary "${SMALL_VALUE}" > /dev/null

SMALL_READ="$(curl -fsS "${CACHE_URL}/cache/${KEY}-small")"

if [ "${SMALL_READ}" != "${SMALL_VALUE}" ]; then
    echo "FAIL: cache size limit sanity check failed"
    exit 1
fi

set +e
PUT_CODE="$(curl -s -o /dev/null -w "%{http_code}" \
  -X PUT "${CACHE_URL}/cache/${KEY}-oversized" \
  --data-binary @"$TMP_BIG")"
PUT_EXIT=$?
set -e

# curl may return 18 because server rejects/closes oversized upload early.
if [ "${PUT_EXIT}" != "0" ] && [ "${PUT_EXIT}" != "18" ]; then
    echo "FAIL: unexpected curl exit for oversized PUT: ${PUT_EXIT}, http=${PUT_CODE}"
    exit 1
fi

GET_CODE="$(curl -s -o "$TMP_GET" -w "%{http_code}" \
  "${CACHE_URL}/cache/${KEY}-oversized")"

if [ "${GET_CODE}" != "404" ]; then
    echo "FAIL: expected oversized value not to be cached"
    echo "expected HTTP 404, got ${GET_CODE}"
    cat "$TMP_GET" || true
    exit 1
fi

echo "cache size limit OK"