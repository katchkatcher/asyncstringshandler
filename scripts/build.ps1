$ErrorActionPreference = "Stop"
Set-Location $PSScriptRoot\..

cmake -S . -B build -G "MinGW Makefiles" `
    -DASYNC_STRINGS_BUILD_EXAMPLES=ON `
    -DASYNC_STRINGS_BUILD_TESTS=ON `
    -DASYNC_STRINGS_BUILD_BENCHMARKS=ON

cmake --build build
