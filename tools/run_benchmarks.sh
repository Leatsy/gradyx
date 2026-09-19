#!/usr/bin/env bash

set -euo pipefail

readonly project_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
readonly build_dir="${project_root}/build/release-bench"
readonly results_path="${build_dir}/benchmark-results.json"

cmake --preset release-bench -S "${project_root}"
cmake --build --preset release-bench
ctest --preset release-bench -L benchmark-smoke
"${build_dir}/benchmarks/gradyx_benchmarks" \
  --benchmark_min_time=0.01s \
  --benchmark_out="${results_path}" \
  --benchmark_out_format=json

echo "benchmark results: ${results_path}"
