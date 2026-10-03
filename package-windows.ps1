$ErrorActionPreference = "Stop"
$projectRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
$bundle = Join-Path $projectRoot "build\KrakenKlipper_artefacts\Release\VST3\DOUBLE CUP CLIPPER.vst3"
$dist = Join-Path $projectRoot "dist"
$sourceStage = Join-Path $dist "source-stage"
$sourceZip = Join-Path $dist "Double-Cup-Clipper-Source-v1.0.2.zip"
$legacySourceZip = Join-Path $dist "Kraken-Klipper-Source.zip"
$setupExe = Join-Path $dist "DOUBLE-CUP-CLIPPER-Setup-v1.0.2.exe"
$setupScript = Join-Path $projectRoot "KRAKEN-KLIPPER.iss"

New-Item -ItemType Directory -Path $dist -Force | Out-Null
$fullDist = [System.IO.Path]::GetFullPath($dist)
$fullSourceStage = [System.IO.Path]::GetFullPath($sourceStage)
$separator = [System.IO.Path]::DirectorySeparatorChar
if (-not $fullSourceStage.StartsWith($fullDist + $separator, [System.StringComparison]::OrdinalIgnoreCase)) {
    throw "Refusing to remove a source staging folder outside dist."
}
if (-not [System.IO.Path]::GetFullPath($legacySourceZip).StartsWith($fullDist + $separator, [System.StringComparison]::OrdinalIgnoreCase)) {
    throw "Refusing to remove a legacy source archive outside dist."
}
if (Test-Path -LiteralPath $legacySourceZip -PathType Leaf) { Remove-Item -LiteralPath $legacySourceZip -Force }
if (Test-Path -LiteralPath $sourceStage) { Remove-Item -LiteralPath $sourceStage -Recurse -Force }
New-Item -ItemType Directory -Path $sourceStage -Force | Out-Null

foreach ($item in @(
    "CMakeLists.txt", "Source", "tests", ".github", "README.txt", "LICENSE.txt",
    "THIRD_PARTY_NOTICES.md", "KRAKEN-KLIPPER.iss", "build-windows.ps1", "package-windows.ps1"
)) {
    $sourceItem = Join-Path $projectRoot $item
    if (-not (Test-Path -LiteralPath $sourceItem)) { throw "Required source item is missing: $sourceItem" }
    Copy-Item -LiteralPath $sourceItem -Destination $sourceStage -Recurse
}

# Enumerate visible and hidden top-level items, then include .github once explicitly.
$sourcePaths = @(
    Get-ChildItem -LiteralPath $sourceStage -Force |
        Where-Object { $_.Name -ne ".github" } |
        ForEach-Object { $_.FullName }
)
$sourcePaths += Join-Path $sourceStage ".github"
Compress-Archive -Path $sourcePaths -DestinationPath $sourceZip -CompressionLevel Optimal -Force
Remove-Item -LiteralPath $sourceStage -Recurse -Force
Write-Host "Created source archive: $sourceZip"

if (-not (Test-Path -LiteralPath $bundle -PathType Container)) {
    throw "Built VST3 bundle not found. Run .\build-windows.ps1 first. Expected: $bundle"
}

$isccCommand = Get-Command ISCC.exe -ErrorAction SilentlyContinue
if ($isccCommand) {
    $iscc = $isccCommand.Source
} else {
    $iscc = Join-Path ${env:ProgramFiles(x86)} "Inno Setup 6\ISCC.exe"
}
if (-not (Test-Path -LiteralPath $iscc -PathType Leaf)) {
    throw "Inno Setup 6 compiler (ISCC.exe) was not found. Install Inno Setup 6, then rerun this script."
}

Push-Location $projectRoot
try {
    & $iscc $setupScript
    if ($LASTEXITCODE -ne 0) { throw "Inno Setup failed to create the installer." }
} finally {
    Pop-Location
}
if (-not (Test-Path -LiteralPath $setupExe -PathType Leaf)) {
    throw "Inno Setup reported success but the installer was not found at $setupExe"
}
Write-Host "Created installer: $setupExe"
