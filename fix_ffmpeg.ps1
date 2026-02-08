# Fix FFmpeg Installation - Download Full Build with Development Files
Write-Host "FFmpeg development dosyalari icin full build indiriliyor..." -ForegroundColor Cyan
Write-Host ""

$ffmpegDir = "C:\ffmpeg"

# Check current installation
if (Test-Path "$ffmpegDir\include\libavcodec\avcodec.h") {
    Write-Host "[OK] FFmpeg zaten dogru sekilde yuklu!" -ForegroundColor Green
    exit 0
}

Write-Host "Mevcut kurulum sadece runtime dosyalari iceriyor." -ForegroundColor Yellow
Write-Host "Development dosyalari icin full build gerekiyor." -ForegroundColor Yellow
Write-Host ""

$response = Read-Host "Mevcut kurulumu temizleyip full build indirmek istiyor musunuz? (Y/N)"
if ($response -ne 'Y' -and $response -ne 'y') {
    Write-Host "Iptal edildi." -ForegroundColor Yellow
    exit 0
}

# Backup bin folder if exists
if (Test-Path "$ffmpegDir\bin") {
    Write-Host "Runtime dosyalari yedekleniyor..." -ForegroundColor Yellow
    $backupDir = "$ffmpegDir-backup-$(Get-Date -Format 'yyyyMMdd-HHmmss')"
    Copy-Item -Path "$ffmpegDir\bin" -Destination "$backupDir\bin" -Recurse -Force -ErrorAction SilentlyContinue
}

# Remove old installation
Write-Host "Eski kurulum temizleniyor..." -ForegroundColor Yellow
Remove-Item -Path $ffmpegDir -Recurse -Force -ErrorAction SilentlyContinue
New-Item -ItemType Directory -Path $ffmpegDir -Force | Out-Null

# Download full build
$urls = @(
    "https://www.gyan.dev/ffmpeg/builds/ffmpeg-release-full.zip",
    "https://www.gyan.dev/ffmpeg/builds/ffmpeg-release-full-shared.zip"
)

$zipFile = Join-Path $env:TEMP "ffmpeg-full.zip"
$extractDir = Join-Path $env:TEMP "ffmpeg-extract"

$downloadSuccess = $false
foreach ($url in $urls) {
    try {
        Write-Host ""
        Write-Host "Indirme basladi: $url" -ForegroundColor Cyan
        Write-Host "(Bu biraz zaman alabilir, dosya ~100MB olabilir)..." -ForegroundColor Yellow
        
        $ProgressPreference = 'SilentlyContinue'
        Invoke-WebRequest -Uri $url -OutFile $zipFile -UseBasicParsing -ErrorAction Stop
        
        Write-Host "[OK] Indirme tamamlandi!" -ForegroundColor Green
        $downloadSuccess = $true
        break
    } catch {
        Write-Host "[HATA] Bu URL basarisiz: $($_.Exception.Message)" -ForegroundColor Red
        if ($urls.IndexOf($url) -lt $urls.Count - 1) {
            Write-Host "Diger URL deneniyor..." -ForegroundColor Yellow
        }
        continue
    }
}

if (-not $downloadSuccess) {
    Write-Host ""
    Write-Host "[HATA] Tum indirme denemeleri basarisiz!" -ForegroundColor Red
    Write-Host ""
    Write-Host "Lutfen manuel olarak indirin:" -ForegroundColor Yellow
    Write-Host "  1. https://www.gyan.dev/ffmpeg/builds/ adresine gidin" -ForegroundColor White
    Write-Host "  2. 'ffmpeg-release-full.zip' dosyasini indirin" -ForegroundColor White
    Write-Host "  3. ZIP dosyasini $ffmpegDir dizinine acin" -ForegroundColor White
    exit 1
}

try {
    Write-Host ""
    Write-Host "Aciliyor..." -ForegroundColor Yellow
    if (Test-Path $extractDir) {
        Remove-Item -Path $extractDir -Recurse -Force
    }
    Expand-Archive -Path $zipFile -DestinationPath $extractDir -Force
    
    # Find the extracted folder
    $extractedFolder = Get-ChildItem -Path $extractDir -Directory | Where-Object { $_.Name -like "ffmpeg-*" } | Select-Object -First 1
    
    if ($extractedFolder) {
        Write-Host "Kopyalaniyor..." -ForegroundColor Yellow
        Write-Host "Kaynak: $($extractedFolder.FullName)" -ForegroundColor Gray
        
        # Copy all contents
        Copy-Item -Path "$($extractedFolder.FullName)\*" -Destination $ffmpegDir -Recurse -Force
        
        # Check for nested structure
        $nestedFolder = Get-ChildItem -Path $ffmpegDir -Directory | Where-Object { $_.Name -like "ffmpeg-*" } | Select-Object -First 1
        if ($nestedFolder -and (Test-Path "$($nestedFolder.FullName)\include\libavcodec\avcodec.h")) {
            Write-Host "Ic ice yapi tespit edildi, duzenleniyor..." -ForegroundColor Yellow
            Move-Item -Path "$($nestedFolder.FullName)\*" -Destination $ffmpegDir -Force
            Remove-Item -Path $nestedFolder.FullName -Force
        }
        
        # Verify installation
        if (Test-Path "$ffmpegDir\include\libavcodec\avcodec.h") {
            Write-Host ""
            Write-Host "[SUCCESS] FFmpeg basariyla yuklendi!" -ForegroundColor Green
            Write-Host "Konum: $ffmpegDir" -ForegroundColor Cyan
            Write-Host "Include: $ffmpegDir\include" -ForegroundColor Gray
            Write-Host "Lib: $ffmpegDir\lib" -ForegroundColor Gray
            Write-Host ""
            Write-Host "CMake'i yeniden yapilandirin." -ForegroundColor Yellow
        } else {
            Write-Host ""
            Write-Host "[HATA] FFmpeg dosyalari dogru yuklenmedi!" -ForegroundColor Red
            Write-Host "C:\ffmpeg icerigi:" -ForegroundColor Yellow
            Get-ChildItem -Path $ffmpegDir | Select-Object Name, PSIsContainer | Format-Table
        }
    } else {
        Write-Host "[HATA] Acilan dizin bulunamadi!" -ForegroundColor Red
        Write-Host "Extract dizini icerigi:" -ForegroundColor Yellow
        Get-ChildItem -Path $extractDir | Select-Object Name, PSIsContainer | Format-Table
    }
    
    # Cleanup
    Remove-Item -Path $zipFile -Force -ErrorAction SilentlyContinue
    Remove-Item -Path $extractDir -Recurse -Force -ErrorAction SilentlyContinue
    
} catch {
    Write-Host "[HATA] Acilma/kopyalama basarisiz: $($_.Exception.Message)" -ForegroundColor Red
    exit 1
}

Write-Host ""
