#!/usr/bin/env bash
set -euo pipefail

run_test() {
  local name="$1"
  local script="$2"

  echo
  echo "============================================================"
  echo " RUNNING: $name"
  echo " SCRIPT : $script"
  echo "============================================================"

  "$script"

  echo "------------------------------------------------------------"
  echo " PASSED : $name"
  echo "------------------------------------------------------------"
}

echo
echo "==================== DFS SMOKE TEST SUITE ===================="

run_test "CACHE" ./scripts/test_cache.sh
run_test "CACHE_STATS" ./scripts/test_cache_stats.sh
run_test "STORAGE" ./scripts/test_storage.sh
run_test "METADATA" ./scripts/test_metadata.sh
run_test "SPLIT" ./scripts/test_split.sh
run_test "E2E" ./scripts/test_e2e.sh
run_test "INVALIDATION" ./scripts/test_invalidation.sh
run_test "SMALL FILE CACHE HIT" ./scripts/test_small_file.sh
run_test "LARGE FILE" ./scripts/test_large_file.sh

echo
echo "==================== ALL TESTS PASSED ===================="
