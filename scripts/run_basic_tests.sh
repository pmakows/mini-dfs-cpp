#!/usr/bin/env bash
set -euo pipefail

./scripts/test_storage.sh
./scripts/test_metadata.sh
./scripts/test_split.sh
./scripts/test_cache.sh

echo
echo "ALL BASIC TESTS PASSED"
