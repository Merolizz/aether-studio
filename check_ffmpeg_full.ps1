# Check FFmpeg Full Installation
Write-Host "FFmpeg tam kurulum kontrol ediliyor..." -ForegroundColor Cyan
Write-Host ""

# Check C:\ffmpeg
Write-Host "=== C:\ffmpeg Kontrolu ===" -ForegroundColor Yellow
if (Test-Path "C:\ffmpeg") {
    Write-Host "[OK] C:\ffmpeg dizini var" -ForegroundColor Green
    if (Test-Path "C:\ffmpeg\include\libavcodec\avcodec.h") {
        Write-Host "[OK] Include dosyalari bulundu!" -ForegroundColor Green
        Write-Host "     Konum: C:\ffmpeg\include" -ForegroundColor Gray
    } else {
        Write-Host "[HATA] Include dosyalari yok" -ForegroundColor Red
    }
    if (Test-Path "C:\ffmpeg\lib") {
        $libFiles = Get-ChildItem "C:\ffmpeg\lib" -Filter "*.lib" -ErrorAction SilentlyContinue
        if ($libFiles) {
            Write-Host "[OK] Lib dosyalari bulundu! ($($libFiles.Count) dosya)" -ForegroundColor Green
            Write-Host "     Konum: C:\ffmpeg\lib" -ForegroundColor Gray
        } else {
            Write-Host "[HATA] Lib dosyalari yok" -ForegroundColor Red
        }
    } else {
        Write-Host "[HATA] Lib klasoru yok" -ForegroundColor Red
    }
} else {
    Write-Host "[HATA] C:\ffmpeg dizini yok" -ForegroundColor Red
}

Write-Host ""

# Check PATH location
Write-Host "=== PATH'teki FFmpeg Kontrolu ===" -ForegroundColor Yellow
$ffmpegExe = Get-Command ffmpeg -ErrorAction SilentlyContinue
if ($ffmpegExe) {
    Write-Host "[OK] FFmpeg PATH'te bulundu" -ForegroundColor Green
    Write-Host "     Konum: $($ffmpegExe.Source)" -ForegroundColor Gray
    
    $binDir = Split-Path $ffmpegExe.Source
    $rootDir = Split-Path $binDir
    
    Write-Host ""
    Write-Host "Kok dizin: $rootDir" -ForegroundColor Cyan
    
    if (Test-Path (Join-Path $rootDir "include\libavcodec\avcodec.h")) {
        Write-Host "[OK] Include dosyalari bulundu!" -ForegroundColor Green
        Write-Host "     Konum: $(Join-Path $rootDir 'include')" -ForegroundColor Gray
    } else {
        Write-Host "[HATA] Include dosyalari yok" -ForegroundColor Red
    }
    
    if (Test-Path (Join-Path $rootDir "lib")) {
        $libFiles = Get-ChildItem (Join-Path $rootDir "lib") -Filter "*.lib" -ErrorAction SilentlyContinue
        if ($libFiles) {
            Write-Host "[OK] Lib dosyalari bulundu! ($($libFiles.Count) dosya)" -ForegroundColor Green
            Write-Host "     Konum: $(Join-Path $rootDir 'lib')" -ForegroundColor Gray
        } else {
            Write-Host "[HATA] Lib dosyalari yok" -ForegroundColor Red
        }
    } else {
        Write-Host "[HATA] Lib klasoru yok" -ForegroundColor Red
    }
} else {
    Write-Host "[HATA] FFmpeg PATH'te bulunamadi" -ForegroundColor Red
}

Write-Host ""
Write-Host "=== Sonuc ===" -ForegroundColor Cyan
Write-Host "CMake bu konumlari otomatik olarak kontrol edecek." -ForegroundColor Yellow
Write-Host ""
