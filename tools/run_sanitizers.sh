#!/usr/bin/env bash

set -euo pipefail

readonly project_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

cmake --preset asan-ubsan -S "${project_root}"
cmake --build --preset asan-ubsan
ctest --preset asan-ubsan

cmake --preset tsan -S "${project_root}"
cmake --build --preset tsan
ctest --preset tsan
