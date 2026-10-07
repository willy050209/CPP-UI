# =============================================================================
# CPPUI Skia Setup Automation Script
# Downloads and prepares the prebuilt Skia (m151) library for Windows x64
# =============================================================================

param(
    [string]$SkiaVersion = "m151-a90155cff0",
    [string]$DestinationDir = "$PSScriptRoot\..\third_party\skia"
)

$targetDir = [System.IO.Path]::GetFullPath($DestinationDir)
$headerCheck = Join-Path $targetDir "include\core\SkCanvas.h"
$libCheck = Join-Path $targetDir "out\Release-x64\skia.lib"

if ((Test-Path $headerCheck) -and (Test-Path $libCheck)) {
    Write-Host "[Skia Setup] Skia $SkiaVersion is already installed at: $targetDir" -ForegroundColor Green
    exit 0
}

Write-Host "==========================================================" -ForegroundColor Cyan
Write-Host " CPPUI Skia Dependency Setup: Downloading Skia $SkiaVersion" -ForegroundColor Cyan
Write-Host "==========================================================" -ForegroundColor Cyan

$tempZip = Join-Path $env:TEMP "Skia-Windows-Release-x64-$SkiaVersion.zip"
$downloadUrl = "https://github.com/aseprite/skia/releases/download/$SkiaVersion/Skia-Windows-Release-x64.zip"

Write-Host "[1/3] Downloading prebuilt Skia archive..."
Write-Host "      URL: $downloadUrl"
Write-Host "      Destination: $tempZip"

if (-not (Test-Path $tempZip) -or ((Get-Item $tempZip).Length -lt 10000000)) {
    curl.exe -L -o "$tempZip" "$downloadUrl"
    if ($LASTEXITCODE -ne 0 -or -not (Test-Path $tempZip)) {
        Write-Error "Failed to download Skia from $downloadUrl"
        exit 1
    }
}

Write-Host "[2/3] Extracting archive to $targetDir..."
if (-not (Test-Path $targetDir)) {
    New-Item -ItemType Directory -Path $targetDir -Force | Out-Null
}

Expand-Archive -Path $tempZip -DestinationPath $targetDir -Force

Write-Host "[3/3] Verifying installation files..."
if ((Test-Path $headerCheck) -and (Test-Path $libCheck)) {
    Write-Host "[Skia Setup] Skia $SkiaVersion successfully installed at: $targetDir" -ForegroundColor Green
    Write-Host "  - Headers: $headerCheck"
    Write-Host "  - Library: $libCheck"
} else {
    Write-Warning "Installation finished, but expected files were not found at standard paths. Please verify $targetDir."
}
