#!/usr/bin/env bash

set -euo pipefail

readonly project_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

cmake --preset asan-ubsan -S "${project_root}"
cmake --build --preset asan-ubsan
ctest --preset asan-ubsan

cmake --preset tsan -S "${project_root}"
cmake --build --preset tsan
if setarch "$(uname -m)" -R true 2>/dev/null; then
  setarch "$(uname -m)" -R ctest --preset tsan
else
  echo "warning: cannot disable ASLR; skipping TSan execution" >&2
  echo "warning: run on a host that allows setarch -R to execute TSan" >&2
fi
