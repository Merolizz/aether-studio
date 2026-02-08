# FFmpeg and OpenSSL Detection Script
Write-Host "FFmpeg ve OpenSSL araniyor..." -ForegroundColor Cyan
Write-Host ""

# Check FFmpeg
Write-Host "=== FFmpeg Kontrolu ===" -ForegroundColor Yellow
$ffmpegPaths = @(
    "C:\ffmpeg",
    "$env:ProgramFiles\ffmpeg",
    "${env:ProgramFiles(x86)}\ffmpeg",
    "C:\vcpkg\installed\x64-windows",
    "$env:VCPKG_ROOT\installed\x64-windows"
)

$ffmpegFound = $false
foreach ($path in $ffmpegPaths) {
    if (Test-Path "$path\include\libavcodec\avcodec.h") {
        Write-Host "[OK] FFmpeg bulundu: $path" -ForegroundColor Green
        Write-Host "     Include: $path\include" -ForegroundColor Gray
        Write-Host "     Lib: $path\lib" -ForegroundColor Gray
        $ffmpegFound = $true
        break
    }
}

if (-not $ffmpegFound) {
    # Check if ffmpeg is in PATH
    $ffmpegExe = Get-Command ffmpeg -ErrorAction SilentlyContinue
    if ($ffmpegExe) {
        Write-Host "[INFO] FFmpeg PATH'te bulundu: $($ffmpegExe.Source)" -ForegroundColor Yellow
        Write-Host "       Ancak development dosyalari (include/lib) bulunamadi." -ForegroundColor Yellow
        Write-Host "       CMake development dosyalarini bulamayabilir." -ForegroundColor Yellow
    } else {
        Write-Host "[HATA] FFmpeg bulunamadi!" -ForegroundColor Red
    }
}

Write-Host ""

# Check OpenSSL
Write-Host "=== OpenSSL Kontrolu ===" -ForegroundColor Yellow
$opensslPaths = @(
    "C:\OpenSSL-Win64",
    "C:\OpenSSL",
    "$env:ProgramFiles\OpenSSL-Win64",
    "${env:ProgramFiles(x86)}\OpenSSL-Win32",
    "C:\vcpkg\installed\x64-windows",
    "$env:VCPKG_ROOT\installed\x64-windows"
)

$opensslFound = $false
foreach ($path in $opensslPaths) {
    if (Test-Path "$path\include\openssl\ssl.h") {
        Write-Host "[OK] OpenSSL bulundu: $path" -ForegroundColor Green
        Write-Host "     Include: $path\include" -ForegroundColor Gray
        Write-Host "     Lib: $path\lib" -ForegroundColor Gray
        $opensslFound = $true
        break
    }
}

if (-not $opensslFound) {
    # Check if openssl is in PATH
    $opensslExe = Get-Command openssl -ErrorAction SilentlyContinue
    if ($opensslExe) {
        Write-Host "[INFO] OpenSSL PATH'te bulundu: $($opensslExe.Source)" -ForegroundColor Yellow
        Write-Host "       Ancak development dosyalari (include/lib) bulunamadi." -ForegroundColor Yellow
        Write-Host "       CMake development dosyalarini bulamayabilir." -ForegroundColor Yellow
    } else {
        Write-Host "[HATA] OpenSSL bulunamadi!" -ForegroundColor Red
    }
}

Write-Host ""
Write-Host "=== Oneriler ===" -ForegroundColor Cyan
if (-not $ffmpegFound) {
    Write-Host "FFmpeg yuklemek icin:" -ForegroundColor Yellow
    Write-Host "  1. https://ffmpeg.org/download.html adresinden indirin" -ForegroundColor White
    Write-Host "  2. C:\ffmpeg dizinine acin" -ForegroundColor White
    Write-Host "  3. Veya vcpkg kullanin: vcpkg install ffmpeg" -ForegroundColor White
}

if (-not $opensslFound) {
    Write-Host "OpenSSL yuklemek icin:" -ForegroundColor Yellow
    Write-Host "  1. https://slproweb.com/products/Win32OpenSSL.html adresinden indirin" -ForegroundColor White
    Write-Host "  2. C:\OpenSSL-Win64 dizinine yukleyin" -ForegroundColor White
    Write-Host "  3. Veya vcpkg kullanin: vcpkg install openssl" -ForegroundColor White
}

Write-Host ""
