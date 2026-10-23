#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")/.."

cmake -S . -B build \
    -DASYNC_STRINGS_BUILD_EXAMPLES=ON \
    -DASYNC_STRINGS_BUILD_TESTS=ON \
    -DASYNC_STRINGS_BUILD_BENCHMARKS=ON

cmake --build build
