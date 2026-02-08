# FFmpeg Development Files Download and Install Script
Write-Host "FFmpeg development dosyalari indiriliyor..." -ForegroundColor Cyan
Write-Host ""

$projectRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
$ffmpegDir = "C:\ffmpeg"

# Check if already installed
if (Test-Path "$ffmpegDir\include\libavcodec\avcodec.h") {
    Write-Host "[INFO] FFmpeg zaten yuklu: $ffmpegDir" -ForegroundColor Green
    $response = Read-Host "Yeniden yuklemek istiyor musunuz? (Y/N)"
    if ($response -ne 'Y' -and $response -ne 'y') {
        Write-Host "Iptal edildi." -ForegroundColor Yellow
        exit 0
    }
    Write-Host "Eski kurulum temizleniyor..." -ForegroundColor Yellow
    Remove-Item -Path $ffmpegDir -Recurse -Force -ErrorAction SilentlyContinue
}

# Create ffmpeg directory
if (-not (Test-Path $ffmpegDir)) {
    New-Item -ItemType Directory -Path $ffmpegDir -Force | Out-Null
}

Write-Host "FFmpeg development build araniyor..." -ForegroundColor Yellow
Write-Host ""
Write-Host "FFmpeg development dosyalari icin birkaç secenek var:" -ForegroundColor Cyan
Write-Host ""
Write-Host "1. GyanFFmpeg (Onerilen - Windows icin hazir build):" -ForegroundColor Yellow
Write-Host "   https://www.gyan.dev/ffmpeg/builds/" -ForegroundColor White
Write-Host "   'ffmpeg-release-essentials.zip' dosyasini indirin" -ForegroundColor White
Write-Host ""
Write-Host "2. BtbN FFmpeg Build:" -ForegroundColor Yellow
Write-Host "   https://github.com/BtbN/FFmpeg-Builds/releases" -ForegroundColor White
Write-Host "   'ffmpeg-master-latest-win64-gpl-shared.zip' dosyasini indirin" -ForegroundColor White
Write-Host ""
Write-Host "3. Manuel indirme:" -ForegroundColor Yellow
Write-Host "   Yukaridaki linklerden birinden ZIP dosyasini indirin" -ForegroundColor White
Write-Host "   Ve $ffmpegDir dizinine acin" -ForegroundColor White
Write-Host ""

$response = Read-Host "Otomatik indirme yapmak istiyor musunuz? (Y/N)"

if ($response -eq 'Y' -or $response -eq 'y') {
    Write-Host ""
    Write-Host "GyanFFmpeg'den indiriliyor..." -ForegroundColor Yellow
    
    # Try to download from GyanFFmpeg - use full build for development files
    # First try full build, then fallback to shared
    $urls = @(
        "https://www.gyan.dev/ffmpeg/builds/ffmpeg-release-full.zip",
        "https://www.gyan.dev/ffmpeg/builds/ffmpeg-release-full-shared.zip"
    )
    $zipFile = Join-Path $env:TEMP "ffmpeg-dev.zip"
    $extractDir = Join-Path $env:TEMP "ffmpeg-extract"
    
    $downloadSuccess = $false
    foreach ($url in $urls) {
        try {
            Write-Host "Indirme basladi: $url" -ForegroundColor Cyan
            Write-Host "(Bu biraz zaman alabilir, dosya buyuk olabilir)..." -ForegroundColor Yellow
            Invoke-WebRequest -Uri $url -OutFile $zipFile -UseBasicParsing -ErrorAction Stop
            $downloadSuccess = $true
            break
        } catch {
            Write-Host "Bu URL basarisiz, digerini deniyor..." -ForegroundColor Yellow
            continue
        }
    }
    
    if (-not $downloadSuccess) {
        Write-Host "[HATA] Indirme basarisiz!" -ForegroundColor Red
        Write-Host ""
        Write-Host "Lutfen manuel olarak indirin:" -ForegroundColor Yellow
        Write-Host "  1. https://www.gyan.dev/ffmpeg/builds/ adresine gidin" -ForegroundColor White
        Write-Host "  2. 'ffmpeg-release-full.zip' veya 'ffmpeg-release-full-shared.zip' dosyasini indirin" -ForegroundColor White
        Write-Host "  3. ZIP dosyasini $ffmpegDir dizinine acin" -ForegroundColor White
        exit 1
    }
    
    try {
        
        Write-Host "Aciliyor..." -ForegroundColor Yellow
        if (Test-Path $extractDir) {
            Remove-Item -Path $extractDir -Recurse -Force
        }
        Expand-Archive -Path $zipFile -DestinationPath $extractDir -Force
        
        # Find the extracted folder - GyanFFmpeg structure
        $extractedFolder = Get-ChildItem -Path $extractDir -Directory | Where-Object { $_.Name -like "ffmpeg-*" } | Select-Object -First 1
        
        if ($extractedFolder) {
            Write-Host "Kopyalaniyor..." -ForegroundColor Yellow
            Write-Host "Kaynak: $($extractedFolder.FullName)" -ForegroundColor Gray
            
            # Check structure - GyanFFmpeg has bin, include, lib folders directly
            if (Test-Path "$($extractedFolder.FullName)\bin") {
                # Copy all contents (bin, include, lib, etc.)
                Copy-Item -Path "$($extractedFolder.FullName)\*" -Destination $ffmpegDir -Recurse -Force
                Write-Host "Dosyalar kopyalandi" -ForegroundColor Gray
            } else {
                # Maybe the structure is different, try copying the whole folder
                Write-Host "Alternatif yapi tespit edildi, tum icerik kopyalaniyor..." -ForegroundColor Yellow
                Copy-Item -Path "$($extractedFolder.FullName)\*" -Destination $ffmpegDir -Recurse -Force
            }
            
            # Verify installation - check multiple possible locations
            $verified = $false
            if (Test-Path "$ffmpegDir\include\libavcodec\avcodec.h") {
                $verified = $true
            } elseif (Test-Path "$ffmpegDir\ffmpeg-*\include\libavcodec\avcodec.h") {
                # Nested structure
                $nestedFolder = Get-ChildItem -Path $ffmpegDir -Directory | Where-Object { $_.Name -like "ffmpeg-*" } | Select-Object -First 1
                if ($nestedFolder) {
                    Write-Host "Ic ice yapi tespit edildi, duzenleniyor..." -ForegroundColor Yellow
                    Move-Item -Path "$($nestedFolder.FullName)\*" -Destination $ffmpegDir -Force
                    Remove-Item -Path $nestedFolder.FullName -Force
                    if (Test-Path "$ffmpegDir\include\libavcodec\avcodec.h") {
                        $verified = $true
                    }
                }
            }
            
            if ($verified) {
                Write-Host ""
                Write-Host "[SUCCESS] FFmpeg basariyla yuklendi!" -ForegroundColor Green
                Write-Host "Konum: $ffmpegDir" -ForegroundColor Cyan
                Write-Host "Include: $ffmpegDir\include" -ForegroundColor Gray
                Write-Host "Lib: $ffmpegDir\lib" -ForegroundColor Gray
                Write-Host ""
                Write-Host "CMake'i yeniden yapilandirin." -ForegroundColor Yellow
            } else {
                Write-Host "[HATA] FFmpeg dosyalari dogru yuklenmedi!" -ForegroundColor Red
                Write-Host "Kontrol ediliyor..." -ForegroundColor Yellow
                Write-Host "C:\ffmpeg icerigi:" -ForegroundColor Cyan
                Get-ChildItem -Path $ffmpegDir | Select-Object Name, PSIsContainer | Format-Table
                Write-Host ""
                Write-Host "Lutfen manuel olarak kontrol edin ve gerekirse duzenleyin." -ForegroundColor Yellow
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
        Write-Host "[HATA] Indirme basarisiz: $($_.Exception.Message)" -ForegroundColor Red
        Write-Host ""
        Write-Host "Lutfen manuel olarak indirin:" -ForegroundColor Yellow
        Write-Host "  1. https://www.gyan.dev/ffmpeg/builds/ adresine gidin" -ForegroundColor White
        Write-Host "  2. 'ffmpeg-release-essentials.zip' dosyasini indirin" -ForegroundColor White
        Write-Host "  3. ZIP dosyasini $ffmpegDir dizinine acin" -ForegroundColor White
        exit 1
    }
} else {
    Write-Host ""
    Write-Host "Manuel kurulum talimatlari:" -ForegroundColor Cyan
    Write-Host "  1. https://www.gyan.dev/ffmpeg/builds/ adresine gidin" -ForegroundColor White
    Write-Host "  2. 'ffmpeg-release-essentials.zip' dosyasini indirin" -ForegroundColor White
    Write-Host "  3. ZIP dosyasini $ffmpegDir dizinine acin" -ForegroundColor White
    Write-Host "  4. CMake'i yeniden yapilandirin" -ForegroundColor White
}

Write-Host ""
