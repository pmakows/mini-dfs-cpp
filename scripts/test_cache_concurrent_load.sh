#!/usr/bin/env bash
set -euo pipefail

CACHE_URL="${CACHE_URL:-http://localhost:9100}"

CLIENTS=100
REQUESTS_PER_CLIENT=100

echo "== cache concurrent load test =="
echo "clients=${CLIENTS}, requests_per_client=${REQUESTS_PER_CLIENT}"

curl -fsS "${CACHE_URL}/health" > /dev/null
curl -fsS -X POST "${CACHE_URL}/reset-stats" > /dev/null

PREFIX="concurrent-load-$(date +%s%N)"
TMP_DIR="$(mktemp -d)"

cleanup() {
    rm -rf "$TMP_DIR"
}
trap cleanup EXIT

for c in $(seq 1 "$CLIENTS"); do
(
    ok=0
    miss=0
    fail=0

    for r in $(seq 1 "$REQUESTS_PER_CLIENT"); do
        KEY="${PREFIX}-${c}-${r}"
        VALUE="value-${c}-${r}"

        if ! curl -fsS -X PUT "${CACHE_URL}/cache/${KEY}" \
          --data-binary "${VALUE}" > /dev/null; then
            fail=$((fail + 1))
            continue
        fi

        BODY_FILE="${TMP_DIR}/body-${c}-${r}"
        CODE="$(curl -s -o "$BODY_FILE" -w "%{http_code}" \
          "${CACHE_URL}/cache/${KEY}")"

        if [ "$CODE" = "200" ]; then
            READ="$(cat "$BODY_FILE")"

            if [ "$READ" = "$VALUE" ]; then
                ok=$((ok + 1))
            else
                echo "FAIL: wrong value for key=${KEY}, expected=${VALUE}, got=${READ}"
                exit 1
            fi
        elif [ "$CODE" = "404" ]; then
            miss=$((miss + 1))
        else
            echo "FAIL: unexpected HTTP code=${CODE} for key=${KEY}"
            exit 1
        fi
    done

    echo "$ok $miss $fail" > "${TMP_DIR}/result-${c}"
) &
done

wait

TOTAL_OK=0
TOTAL_MISS=0
TOTAL_FAIL=0

for file in "${TMP_DIR}"/result-*; do
    read -r ok miss fail < "$file"
    TOTAL_OK=$((TOTAL_OK + ok))
    TOTAL_MISS=$((TOTAL_MISS + miss))
    TOTAL_FAIL=$((TOTAL_FAIL + fail))
done

EXPECTED=$((CLIENTS * REQUESTS_PER_CLIENT))

STATS="$(curl -fsS "${CACHE_URL}/stats")"
ANALYZE="$(curl -fsS "${CACHE_URL}/analyze")"

echo "$STATS"
echo "$ANALYZE"
echo "ok=${TOTAL_OK}, miss=${TOTAL_MISS}, fail=${TOTAL_FAIL}, expected=${EXPECTED}"

if [ "$TOTAL_FAIL" != "0" ]; then
    echo "FAIL: some PUT requests failed"
    exit 1
fi

if [ $((TOTAL_OK + TOTAL_MISS)) != "$EXPECTED" ]; then
    echo "FAIL: not all GET requests completed"
    exit 1
fi

curl -fsS "${CACHE_URL}/health" > /dev/null

echo "cache concurrent load OK"