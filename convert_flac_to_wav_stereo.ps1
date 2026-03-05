# ============================================================
# convert_flac_to_wav_stereo.ps1
# ============================================================
# Converts all FLAC files to WAV stereo 24-bit 48000Hz.
# Run this AFTER placing the original Salamander FLAC files
# in sfz/Samples/ (re-downloaded from sfzinstruments.github.io).
#
# Result: stereo 24-bit 48kHz WAV - best quality, ~1.2 GB
# ============================================================

$samplesDir = "c:\Users\remi\Desktop\SalamanderPianoApp\sfz\Samples"

$flacFiles = @(Get-ChildItem -Path $samplesDir -Recurse -Filter "*.flac")
$total = $flacFiles.Count

if ($total -eq 0) {
    Write-Host "Aucun fichier FLAC trouve dans $samplesDir" -ForegroundColor Red
    Write-Host "Telechargez d'abord les samples depuis :"
    Write-Host "  https://freepats.zenvoid.org/Piano/acoustic-grand-piano.html" -ForegroundColor Cyan
    Write-Host "  (Salamander Grand Piano V3 - FLAC)" -ForegroundColor Cyan
    exit 1
}

Write-Host "=== FLAC -> WAV stereo 24-bit 48kHz ===" -ForegroundColor Cyan
Write-Host "Fichiers: $total" -ForegroundColor Cyan
Write-Host ""

$i = 0
$errors = 0

foreach ($file in $flacFiles) {
    $i++
    $wavPath = [System.IO.Path]::ChangeExtension($file.FullName, "wav")
    $tmp = $wavPath + ".tmp.wav"

    Write-Progress -Activity "Conversion en cours..." `
                   -Status "$i / $total : $($file.Name)" `
                   -PercentComplete ([int](($i / $total) * 100))

    # stereo (-ac 2), 24-bit (-sample_fmt s24), 48kHz, soxr resampler
    $null = & ffmpeg -y -i $file.FullName `
        -ar 48000 -ac 2 -sample_fmt s32 -acodec pcm_s24le `
        -af "aresample=resampler=soxr" `
        $tmp 2>&1

    if ($LASTEXITCODE -eq 0) {
        Remove-Item $file.FullName -Force
        if (Test-Path $wavPath) { Remove-Item $wavPath -Force }
        Rename-Item $tmp $wavPath
    } else {
        Write-Host "ERREUR sur $($file.Name)" -ForegroundColor Red
        if (Test-Path $tmp) { Remove-Item $tmp -Force }
        $errors++
    }
}

Write-Progress -Activity "Conversion en cours..." -Completed

$wavFiles = @(Get-ChildItem -Path $samplesDir -Recurse -Filter "*.wav")
$totalSize = ($wavFiles | Measure-Object -Property Length -Sum).Sum / 1MB
Write-Host ""
Write-Host "=== Termine ===" -ForegroundColor Cyan
Write-Host "Fichiers: $($wavFiles.Count) WAV stereo 24-bit 48kHz" -ForegroundColor Green
Write-Host "Taille  : $([math]::Round($totalSize, 0)) MB" -ForegroundColor Green
if ($errors -gt 0) {
    Write-Host "Erreurs : $errors" -ForegroundColor Red
} else {
    Write-Host "Aucune erreur" -ForegroundColor Green
}
