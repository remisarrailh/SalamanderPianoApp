# ============================================================
# build_android.ps1 - Build SalamanderPiano APK for Android
# ============================================================
# Usage:
#   .\build_android.ps1                  # Debug APK (arm64-v8a)
#   .\build_android.ps1 -Release         # Release APK (signed with debug key)
#   .\build_android.ps1 -Clean           # Clean before building
#
# Prerequisites:
#   - Android Studio or Android SDK (set ANDROID_HOME or sdk.dir in android/local.properties)
#   - Android NDK 28.x installed via SDK Manager
#   - JDK 17+
#   - On first run: Windows build must have been done once to generate juceaide.exe
#     (run .\build_windows.ps1 first)
#
# Output:
#   android/app/build/outputs/apk/debug_/debug/   - Debug APK
#   android/app/build/outputs/apk/release_/release/ - Release APK
# ============================================================

param(
    [switch]$Release,
    [switch]$Clean
)

$ErrorActionPreference = "Stop"
$projectRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
$androidDir  = Join-Path $projectRoot "android"

# ── Locate gradlew ────────────────────────────────────────
$gradlew = Join-Path $androidDir "gradlew.bat"
if (-not (Test-Path $gradlew)) {
    Write-Host "ERROR: $gradlew not found." -ForegroundColor Red
    exit 1
}

# ── Verify local.properties exists ────────────────────────
$localProps = Join-Path $androidDir "local.properties"
if (-not (Test-Path $localProps)) {
    $sdkPath = $env:ANDROID_HOME
    if (-not $sdkPath) {
        $sdkPath = "$env:LOCALAPPDATA\Android\Sdk"
    }
    if (Test-Path $sdkPath) {
        $escaped = $sdkPath.Replace("\", "\\")
        "sdk.dir=$escaped" | Set-Content $localProps -Encoding UTF8
        Write-Host "Created local.properties with sdk.dir=$sdkPath" -ForegroundColor Yellow
    } else {
        Write-Host "ERROR: Android SDK not found. Set ANDROID_HOME or create android\local.properties" -ForegroundColor Red
        Write-Host "       Example: sdk.dir=C:\\Users\\YourName\\AppData\\Local\\Android\\Sdk" -ForegroundColor Yellow
        exit 1
    }
}

Write-Host "============================================" -ForegroundColor Cyan
Write-Host " SalamanderPiano Android Build" -ForegroundColor Cyan
Write-Host " Mode: $(if ($Release) { 'Release' } else { 'Debug' })" -ForegroundColor Cyan
Write-Host "============================================" -ForegroundColor Cyan
Write-Host ""

Push-Location $androidDir

# ── Clean ─────────────────────────────────────────────────
if ($Clean) {
    Write-Host "[1/2] Cleaning..." -ForegroundColor Yellow
    & $gradlew clean
    if ($LASTEXITCODE -ne 0) { Pop-Location; Write-Host "ERROR: Clean failed." -ForegroundColor Red; exit 1 }
    Write-Host "       Done." -ForegroundColor Green
} else {
    Write-Host "[1/2] Clean: skipped" -ForegroundColor DarkGray
}

# ── Build APK ─────────────────────────────────────────────
if ($Release) {
    Write-Host "[2/2] Building Release APK..." -ForegroundColor Yellow
    & $gradlew assembleRelease_Release
    $apkDir = Join-Path $androidDir "app\build\outputs\apk\release_\release"
} else {
    Write-Host "[2/2] Building Debug APK..." -ForegroundColor Yellow
    & $gradlew assembleDebug_Debug
    $apkDir = Join-Path $androidDir "app\build\outputs\apk\debug_\debug"
}

if ($LASTEXITCODE -ne 0) {
    Pop-Location
    Write-Host "ERROR: Gradle build failed." -ForegroundColor Red
    exit 1
}

Pop-Location

Write-Host ""
Write-Host "============================================" -ForegroundColor Green
Write-Host " Build complete!" -ForegroundColor Green
Write-Host "============================================" -ForegroundColor Green

$apk = Get-ChildItem $apkDir -Filter "*.apk" -ErrorAction SilentlyContinue | Select-Object -First 1
if ($apk) {
    Write-Host "APK: $($apk.FullName)" -ForegroundColor Cyan
    Write-Host ""
    Write-Host "Install on connected device:" -ForegroundColor Yellow
    Write-Host "  adb install `"$($apk.FullName)`"" -ForegroundColor White
} else {
    Write-Host "APK not found in $apkDir" -ForegroundColor Red
}

Write-Host ""
