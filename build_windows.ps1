# ============================================================
# build_windows.ps1 - Build SalamanderPiano for Windows
# ============================================================
# Usage:
#   .\build_windows.ps1                           # Build (Standalone + VST3, Debug)
#   .\build_windows.ps1 -Release                  # Release build
#   .\build_windows.ps1 -Clean                    # Clean + rebuild
#   .\build_windows.ps1 -InstallVst               # Build + copy VST3 to C:\Program Files\Common Files\VST3
#   .\build_windows.ps1 -Package                  # Build + create NSIS installer (requires NSIS)
#   .\build_windows.ps1 -Release -Package         # Release build + installer
#   .\build_windows.ps1 -Release -Package -Publish           # Build + installer + GitHub Release
#   .\build_windows.ps1 -Release -Package -Publish -Tag v1.2 # Custom tag
#
# Prerequisites:
#   - Visual Studio 2022 (or Build Tools) with C++ workload
#   - CMake 3.22+ (included with VS)
#   - NSIS 3.x (https://nsis.sourceforge.io/) for -Package
#   - GitHub CLI (gh) authenticated for -Publish
# ============================================================

param(
    [switch]$Release,
    [switch]$Clean,
    [switch]$InstallVst,
    [switch]$Package,
    [switch]$Publish,
    [string]$Tag = ""
)

$ErrorActionPreference = "Stop"
$projectRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
$buildDir    = Join-Path $projectRoot "build"
$config      = if ($Release) { "Release" } else { "Debug" }

Write-Host "============================================" -ForegroundColor Cyan
Write-Host " SalamanderPiano Windows Build" -ForegroundColor Cyan
Write-Host " Config: $config" -ForegroundColor Cyan
Write-Host "============================================" -ForegroundColor Cyan
Write-Host ""

# ── 1. Clean ──────────────────────────────────────────────
if ($Clean -and (Test-Path $buildDir)) {
    Write-Host "[1/4] Cleaning build directory..." -ForegroundColor Yellow
    Remove-Item -Recurse -Force $buildDir
    Write-Host "       Done." -ForegroundColor Green
} else {
    Write-Host "[1/4] Clean: skipped" -ForegroundColor DarkGray
}

# ── 2. CMake Configure ────────────────────────────────────
if (-not (Test-Path (Join-Path $buildDir "CMakeCache.txt"))) {
    Write-Host "[2/4] Configuring with CMake..." -ForegroundColor Yellow
    cmake -B $buildDir -S $projectRoot
    if ($LASTEXITCODE -ne 0) { Write-Host "ERROR: CMake configure failed." -ForegroundColor Red; exit 1 }
    Write-Host "       Done." -ForegroundColor Green
} else {
    Write-Host "[2/4] Configure: already done (use -Clean to reconfigure)" -ForegroundColor DarkGray
}

# ── 3. Build ──────────────────────────────────────────────
Write-Host "[3/4] Building (Standalone + VST3)..." -ForegroundColor Yellow
cmake --build $buildDir --config $config
if ($LASTEXITCODE -ne 0) { Write-Host "ERROR: Build failed." -ForegroundColor Red; exit 1 }

Write-Host ""
Write-Host "============================================" -ForegroundColor Green
Write-Host " Build complete!" -ForegroundColor Green
Write-Host "============================================" -ForegroundColor Green

$standalonePath = Join-Path $buildDir "SalamanderPiano_artefacts\$config\Standalone\Salamander Piano.exe"
$vst3Path       = Join-Path $buildDir "SalamanderPiano_artefacts\$config\VST3\Salamander Piano.vst3"

if (Test-Path $standalonePath) { Write-Host "Standalone: $standalonePath" -ForegroundColor Cyan }
if (Test-Path $vst3Path)       { Write-Host "VST3:       $vst3Path"       -ForegroundColor Cyan }

# ── 4. Package (NSIS installer) ───────────────────────────
if ($Package) {
    Write-Host ""
    Write-Host "[4/4] Creating installer with CPack + NSIS..." -ForegroundColor Yellow

    $makensis = Get-Command "makensis" -ErrorAction SilentlyContinue
    if (-not $makensis) {
        $nsisDefault = "C:\Program Files (x86)\NSIS\makensis.exe"
        if (Test-Path $nsisDefault) {
            $env:PATH += ";C:\Program Files (x86)\NSIS"
            $makensis = $nsisDefault
        } else {
            Write-Host "ERROR: NSIS (makensis) not found. Install NSIS from https://nsis.sourceforge.io/" -ForegroundColor Red
            exit 1
        }
    }

    Push-Location $buildDir
    cpack -C $config -G NSIS
    if ($LASTEXITCODE -ne 0) { Pop-Location; Write-Host "ERROR: CPack failed." -ForegroundColor Red; exit 1 }
    Pop-Location

    $installer = Get-ChildItem $buildDir -Filter "SalamanderPiano-*.exe" | Sort-Object LastWriteTime -Descending | Select-Object -First 1
    if ($installer) {
        Write-Host "Installer: $($installer.FullName)" -ForegroundColor Green
    }
} else {
    Write-Host "[4/4] Package: skipped (use -Package to create NSIS installer)" -ForegroundColor DarkGray
}

# ── Publish to GitHub Releases ────────────────────────────
if ($Publish) {
    if (-not $Package) {
        Write-Host "ERROR: -Publish requires -Package. Run with both flags." -ForegroundColor Red
        exit 1
    }

    if (-not (Get-Command "gh" -ErrorAction SilentlyContinue)) {
        Write-Host "ERROR: GitHub CLI (gh) not found. Install from https://cli.github.com/" -ForegroundColor Red
        exit 1
    }

    $installer = Get-ChildItem $buildDir -Filter "SalamanderPiano-*.exe" | Sort-Object LastWriteTime -Descending | Select-Object -First 1
    if (-not $installer) {
        Write-Host "ERROR: Installer not found in $buildDir" -ForegroundColor Red
        exit 1
    }

    # Derive tag from installer name (e.g. SalamanderPiano-1.0.0-win64.exe → v1.0.0) or use -Tag
    if (-not $Tag) {
        if ($installer.Name -match "SalamanderPiano-(.+)-win64\.exe") {
            $Tag = "v$($Matches[1])"
        } else {
            $Tag = "v1.0.0"
        }
    }

    Write-Host ""
    Write-Host "Publishing to GitHub Releases as tag $Tag ..." -ForegroundColor Yellow

    # Create or update the release and upload the installer
    $releaseExists = gh release view $Tag --json tagName 2>&1
    if ($LASTEXITCODE -eq 0) {
        Write-Host "  Release $Tag already exists, uploading asset..." -ForegroundColor DarkGray
        gh release upload $Tag $installer.FullName --clobber
    } else {
        gh release create $Tag $installer.FullName `
            --title "Salamander Piano $Tag" `
            --notes "Windows installer (Standalone application + VST3 plugin + SFZ samples)." `
            --latest
    }

    if ($LASTEXITCODE -ne 0) {
        Write-Host "ERROR: GitHub release failed." -ForegroundColor Red
        exit 1
    }

    Write-Host "Published:  https://github.com/remisarrailh/SalamanderPianoApp/releases/tag/$Tag" -ForegroundColor Green
}

# ── Install VST3 if requested ─────────────────────────────
if ($InstallVst) {
    Write-Host ""
    $vstInstallDir = "C:\Program Files\Common Files\VST3"
    if (Test-Path $vst3Path) {
        Write-Host "Installing VST3 to $vstInstallDir ..." -ForegroundColor Yellow
        $tmpScript = [System.IO.Path]::GetTempFileName() + ".ps1"
        $vst3PathEsc      = $vst3Path.Replace("'", "''")
        $vstInstallDirEsc = $vstInstallDir.Replace("'", "''")
        @"
if (-not (Test-Path '$vstInstallDirEsc')) { New-Item -ItemType Directory -Force -Path '$vstInstallDirEsc' | Out-Null }
`$dest = '$vstInstallDirEsc\Salamander Piano.vst3'
if (Test-Path `$dest) { Remove-Item -Recurse -Force `$dest }
Copy-Item -Recurse -Force '$vst3PathEsc' `$dest
"@ | Set-Content $tmpScript -Encoding UTF8
        Start-Process powershell -ArgumentList "-NoProfile -ExecutionPolicy Bypass -File `"$tmpScript`"" -Verb RunAs -Wait
        Remove-Item $tmpScript -ErrorAction SilentlyContinue
        if (Test-Path (Join-Path $vstInstallDir "Salamander Piano.vst3")) {
            Write-Host "Installed:  $vstInstallDir\Salamander Piano.vst3" -ForegroundColor Green
        } else {
            Write-Host "Install may have failed or UAC was cancelled." -ForegroundColor Red
        }
    } else {
        Write-Host "ERROR: VST3 not found at $vst3Path" -ForegroundColor Red
    }
}

Write-Host ""
