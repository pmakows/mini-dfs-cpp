#!/usr/bin/env bash
set -euo pipefail

echo "== RUNNING ALL TESTS =="

echo
./scripts/test_cache.sh

echo
./scripts/test_storage.sh

echo
./scripts/test_metadata.sh

echo
./scripts/test_split.sh

echo
./scripts/test_e2e.sh

echo
./scripts/test_invalidation.sh

echo
./scripts/test_large_file.sh

echo
echo "ALL TESTS PASSED"
