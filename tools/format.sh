#!/usr/bin/env bash

set -euo pipefail

readonly project_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
readonly clang_format="${CLANG_FORMAT:-clang-format}"

if ! command -v "${clang_format}" >/dev/null 2>&1; then
  echo "error: clang-format was not found" >&2
  exit 1
fi

mapfile -d '' source_files < <(
  find \
    "${project_root}/include" \
    "${project_root}/src" \
    "${project_root}/backend" \
    "${project_root}/tests" \
    "${project_root}/benchmarks" \
    -type f \
    \( -name '*.h' -o -name '*.hpp' -o -name '*.c' -o -name '*.cc' -o -name '*.cpp' \) \
    -print0
)

if [[ ${#source_files[@]} -eq 0 ]]; then
  exit 0
fi

if [[ "${1:-}" == "--check" ]]; then
  "${clang_format}" --dry-run --Werror "${source_files[@]}"
else
  "${clang_format}" -i "${source_files[@]}"
fi
