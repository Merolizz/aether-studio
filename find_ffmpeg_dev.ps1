# Find FFmpeg Development Files
$ffmpegPath = "C:\Users\watch\AppData\Local\Microsoft\WinGet\Packages\Gyan.FFmpeg_Microsoft.Winget.Source_8wekyb3d8bbwe\ffmpeg-8.0.1-full_build"

Write-Host "FFmpeg dizin yapisi kontrol ediliyor..." -ForegroundColor Cyan
Write-Host "Konum: $ffmpegPath" -ForegroundColor Yellow
Write-Host ""

if (Test-Path $ffmpegPath) {
    Write-Host "[OK] Dizin bulundu" -ForegroundColor Green
    Write-Host ""
    Write-Host "Ana dizin icerigi:" -ForegroundColor Cyan
    Get-ChildItem $ffmpegPath | Select-Object Name, PSIsContainer | Format-Table
    
    Write-Host ""
    Write-Host "Include klasoru araniyor..." -ForegroundColor Yellow
    $includeDirs = Get-ChildItem $ffmpegPath -Recurse -Directory -Filter "include" -ErrorAction SilentlyContinue
    if ($includeDirs) {
        foreach ($dir in $includeDirs) {
            Write-Host "[BULUNDU] $($dir.FullName)" -ForegroundColor Green
            if (Test-Path (Join-Path $dir.FullName "libavcodec\avcodec.h")) {
                Write-Host "  -> libavcodec/avcodec.h bulundu!" -ForegroundColor Green
            }
        }
    } else {
        Write-Host "[HATA] Include klasoru bulunamadi" -ForegroundColor Red
    }
    
    Write-Host ""
    Write-Host "Lib klasoru araniyor..." -ForegroundColor Yellow
    $libDirs = Get-ChildItem $ffmpegPath -Recurse -Directory -Filter "lib" -ErrorAction SilentlyContinue
    if ($libDirs) {
        foreach ($dir in $libDirs) {
            Write-Host "[BULUNDU] $($dir.FullName)" -ForegroundColor Green
            $libFiles = Get-ChildItem $dir.FullName -Filter "*.lib" -ErrorAction SilentlyContinue
            if ($libFiles) {
                Write-Host "  -> $($libFiles.Count) .lib dosyasi bulundu" -ForegroundColor Green
                $libFiles | Select-Object -First 5 Name | ForEach-Object { Write-Host "     - $($_.Name)" -ForegroundColor Gray }
            }
        }
    } else {
        Write-Host "[HATA] Lib klasoru bulunamadi" -ForegroundColor Red
    }
} else {
    Write-Host "[HATA] Dizin bulunamadi!" -ForegroundColor Red
}

Write-Host ""
