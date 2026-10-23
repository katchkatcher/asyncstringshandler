#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")/.."

if [[ ! -x build/benchmarks/async_strings_handler_benchmark ]]; then
    ./scripts/build.sh
fi

./build/benchmarks/async_strings_handler_benchmark 100
