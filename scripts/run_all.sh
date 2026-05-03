#!/usr/bin/env bash
set -euo pipefail

echo "== RUNNING ALL TESTS =="

echo "[CACHE]"
./scripts/test_cache.sh

echo "[STORAGE]"
./scripts/test_storage.sh

echo "[METADATA]"
./scripts/test_metadata.sh

echo "[SPLIT]"
./scripts/test_split.sh

echo "[E2E]"
./scripts/test_e2e.sh

echo "[INVALIDATION]"
./scripts/test_invalidation.sh

echo "[LARGE FILE]"
./scripts/test_large_file.sh

echo "ALL TESTS PASSED"
