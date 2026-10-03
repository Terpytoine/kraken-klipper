$ErrorActionPreference = "Stop"
$projectRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
$buildDir = Join-Path $projectRoot "build"
$cmakeCommand = Get-Command cmake -ErrorAction SilentlyContinue

if ($cmakeCommand) {
    $cmakeExe = $cmakeCommand.Source
} else {
    $vswhere = Join-Path ([Environment]::GetEnvironmentVariable('ProgramFiles(x86)')) "Microsoft Visual Studio\Installer\vswhere.exe"
    if (Test-Path -LiteralPath $vswhere -PathType Leaf) {
        $cmakeCandidate = & $vswhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.CMake.Project -find "Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" | Select-Object -First 1
        if ($cmakeCandidate -and (Test-Path -LiteralPath $cmakeCandidate -PathType Leaf)) { $cmakeExe = $cmakeCandidate }
    }
}

if (-not $cmakeExe) {
    throw "CMake 3.22+ was not found. Install Visual Studio 2022 with the C++ CMake tools, or install standalone CMake and add it to PATH."
}

& $cmakeExe -S $projectRoot -B $buildDir -G "Visual Studio 17 2022" -A x64 -DBUILD_TESTING=ON
if ($LASTEXITCODE -ne 0) { throw "CMake configuration failed." }

& $cmakeExe --build $buildDir --config Release --parallel
if ($LASTEXITCODE -ne 0) { throw "The Release build failed." }

& $cmakeExe -E env ctest --test-dir $buildDir -C Release --output-on-failure
if ($LASTEXITCODE -ne 0) { throw "The transfer-curve or processor smoke checks failed." }

& (Join-Path $projectRoot "package-windows.ps1")
if ($LASTEXITCODE -ne 0) { throw "The Windows installer packaging step failed." }

Write-Host "Build, transfer-curve checks, processor smoke checks, and installer packaging completed successfully."
