#!/usr/bin/env bash

set -euo pipefail

readonly project_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

cmake --preset lint -S "${project_root}"
cmake --build --preset lint
