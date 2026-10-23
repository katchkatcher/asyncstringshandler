#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")/.."

if [[ ! -f build/CTestTestfile.cmake ]]; then
    ./scripts/build.sh
fi

ctest --test-dir build --output-on-failure
