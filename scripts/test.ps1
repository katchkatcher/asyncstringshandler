$ErrorActionPreference = "Stop"
Set-Location $PSScriptRoot\..

if (-not (Test-Path build\CTestTestfile.cmake)) {
    & $PSScriptRoot\build.ps1
}

ctest --test-dir build --output-on-failure
