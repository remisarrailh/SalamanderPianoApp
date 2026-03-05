$sfzDir    = "c:\Users\remi\Desktop\SalamanderPianoApp\sfz"
$sfzFile   = "$sfzDir\Salamander Grand Piano V3.sfz"

$flacFiles = @(Get-ChildItem -Path $sfzDir -Recurse -Filter "*.flac")
$total     = $flacFiles.Count
$i         = 0
$errors    = 0

Write-Host "=== FLAC -> WAV 16bit 44100Hz ===" -ForegroundColor Cyan
Write-Host "Fichiers: $total | Source: $sfzDir" -ForegroundColor Cyan
Write-Host ""

foreach ($file in $flacFiles) {
    $i++
    $wavPath = [System.IO.Path]::ChangeExtension($file.FullName, ".wav")

    Write-Progress -Activity "Conversion en cours..." `
                   -Status "$i / $total : $($file.Name)" `
                   -PercentComplete ([int](($i / $total) * 100))

    $result = & ffmpeg -y -i $file.FullName `
        -ar 44100 -ac 1 -sample_fmt s16 `
        $wavPath 2>&1

    if ($LASTEXITCODE -eq 0) {
        Remove-Item $file.FullName -Force
    } else {
        Write-Host "ERREUR sur $($file.Name)" -ForegroundColor Red
        Write-Host ($result | Select-String "Error|error" | Select-Object -First 3)
        $errors++
    }
}

Write-Progress -Activity "Conversion en cours..." -Completed

# Patch le fichier SFZ : remplace $EXT flac par $EXT wav
Write-Host ""
Write-Host "Patching SFZ ($sfzFile)..." -ForegroundColor Yellow
$sfzContent = Get-Content $sfzFile -Raw
$patched    = $sfzContent -replace '\$EXT flac', '$EXT wav'
$patched    = $patched    -replace "define \`$EXT flac", "define `$EXT wav"

if ($patched -ne $sfzContent) {
    Set-Content -Path $sfzFile -Value $patched -NoNewline
    Write-Host "SFZ patche : extension flac -> wav" -ForegroundColor Green
} else {
    Write-Host "AVERTISSEMENT: pattern non trouve dans le SFZ, verifier manuellement" -ForegroundColor Red
    Write-Host "Cherche: define `$EXT flac"
    Select-String "EXT|flac|wav" $sfzFile | Select-Object -First 5 -ExpandProperty Line
}

# Stats finales
Write-Host ""
$wavFiles  = @(Get-ChildItem -Path $sfzDir -Recurse -Filter "*.wav")
$totalSize = ($wavFiles | Measure-Object -Property Length -Sum).Sum / 1GB
Write-Host "=== Termine ===" -ForegroundColor Cyan
Write-Host "WAV generes : $($wavFiles.Count) fichiers" -ForegroundColor Green
Write-Host "Taille WAV  : $([math]::Round($totalSize, 2)) GB" -ForegroundColor Green
if ($errors -gt 0) {
    Write-Host "Erreurs     : $errors" -ForegroundColor Red
} else {
    Write-Host "Aucune erreur" -ForegroundColor Green
}
