$sfzDir = "c:\Users\remi\Desktop\SalamanderPianoApp\sfz"

$wavFiles = @(Get-ChildItem -Path $sfzDir -Recurse -Filter "*.wav")
$total    = $wavFiles.Count
$i        = 0
$errors   = 0

Write-Host "=== WAV 44100Hz -> 48000Hz resample ===" -ForegroundColor Cyan
Write-Host "Fichiers: $total" -ForegroundColor Cyan
Write-Host ""

foreach ($file in $wavFiles) {
    $i++
    $tmp = $file.FullName + ".tmp.wav"

    Write-Progress -Activity "Resample en cours..." `
                   -Status "$i / $total : $($file.Name)" `
                   -PercentComplete ([int](($i / $total) * 100))

    # -af aresample=resampler=soxr pour meilleure qualite SRC
    $null = & ffmpeg -y -i $file.FullName `
        -ar 48000 -ac 1 -sample_fmt s16 `
        -af "aresample=resampler=soxr" `
        $tmp 2>&1

    if ($LASTEXITCODE -eq 0) {
        Remove-Item $file.FullName -Force
        Rename-Item $tmp $file.FullName
    } else {
        Write-Host "ERREUR sur $($file.Name)" -ForegroundColor Red
        if (Test-Path $tmp) { Remove-Item $tmp -Force }
        $errors++
    }
}

Write-Progress -Activity "Resample en cours..." -Completed

# Stats finales
$wavFiles2  = @(Get-ChildItem -Path $sfzDir -Recurse -Filter "*.wav")
$totalSize  = ($wavFiles2 | Measure-Object -Property Length -Sum).Sum / 1MB
Write-Host ""
Write-Host "=== Termine ===" -ForegroundColor Cyan
Write-Host "Fichiers: $($wavFiles2.Count) a 48000Hz" -ForegroundColor Green
Write-Host "Taille  : $([math]::Round($totalSize, 0)) MB" -ForegroundColor Green
if ($errors -gt 0) {
    Write-Host "Erreurs : $errors" -ForegroundColor Red
} else {
    Write-Host "Aucune erreur" -ForegroundColor Green
}
