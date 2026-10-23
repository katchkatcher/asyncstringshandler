$ErrorActionPreference = "Stop"
Set-Location $PSScriptRoot\..

if (-not (Test-Path build\benchmarks\async_strings_handler_benchmark.exe)) {
    & $PSScriptRoot\build.ps1
}

& .\build\benchmarks\async_strings_handler_benchmark.exe 100
