#!/usr/bin/env bash

set -euo pipefail

readonly project_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
readonly clang_format="${CLANG_FORMAT:-clang-format}"

if ! command -v "${clang_format}" >/dev/null 2>&1; then
  echo "error: clang-format was not found" >&2
  exit 1
fi

source_dirs=()
for relative_dir in include src backend tests benchmarks; do
  if [[ -d "${project_root}/${relative_dir}" ]]; then
    source_dirs+=("${project_root}/${relative_dir}")
  fi
done

if [[ ${#source_dirs[@]} -eq 0 ]]; then
  exit 0
fi

mapfile -d '' source_files < <(
  find \
    "${source_dirs[@]}" \
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
