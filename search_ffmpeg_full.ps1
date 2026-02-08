# Search for FFmpeg Full Installation with Development Files
Write-Host "FFmpeg full kurulumu araniyor (include ve lib dosyalari ile)..." -ForegroundColor Cyan
Write-Host ""

$foundPaths = @()

# Common installation paths
$searchPaths = @(
    "C:\ffmpeg",
    "C:\Program Files\ffmpeg",
    "C:\Program Files (x86)\ffmpeg",
    "$env:ProgramFiles\ffmpeg",
    "${env:ProgramFiles(x86)}\ffmpeg",
    "$env:LOCALAPPDATA\Microsoft\WinGet\Packages",
    "$env:USERPROFILE\ffmpeg",
    "C:\tools\ffmpeg",
    "D:\ffmpeg",
    "E:\ffmpeg"
)

Write-Host "Yaygin konumlar kontrol ediliyor..." -ForegroundColor Yellow
foreach ($path in $searchPaths) {
    if (Test-Path $path) {
        Write-Host "Kontrol ediliyor: $path" -ForegroundColor Gray
        
        # Check if it's a directory or contains ffmpeg subdirectories
        if (Test-Path "$path\include\libavcodec\avcodec.h") {
            Write-Host "  [BULUNDU!] $path" -ForegroundColor Green
            $foundPaths += $path
        } elseif (Test-Path "$path\lib\avcodec.lib") {
            Write-Host "  [BULUNDU!] $path" -ForegroundColor Green
            $foundPaths += $path
        } else {
            # Check subdirectories
            Get-ChildItem -Path $path -Directory -ErrorAction SilentlyContinue | ForEach-Object {
                $subPath = $_.FullName
                if (Test-Path "$subPath\include\libavcodec\avcodec.h") {
                    Write-Host "  [BULUNDU!] $subPath" -ForegroundColor Green
                    $foundPaths += $subPath
                }
            }
        }
    }
}

# Search in WinGet packages
Write-Host ""
Write-Host "WinGet paketleri kontrol ediliyor..." -ForegroundColor Yellow
$wingetPath = "$env:LOCALAPPDATA\Microsoft\WinGet\Packages"
if (Test-Path $wingetPath) {
    Get-ChildItem -Path $wingetPath -Directory -ErrorAction SilentlyContinue | ForEach-Object {
        $packagePath = $_.FullName
        # Look for ffmpeg directories
        Get-ChildItem -Path $packagePath -Recurse -Directory -Filter "*ffmpeg*" -ErrorAction SilentlyContinue | ForEach-Object {
            $ffmpegDir = $_.FullName
            if (Test-Path "$ffmpegDir\include\libavcodec\avcodec.h") {
                Write-Host "  [BULUNDU!] $ffmpegDir" -ForegroundColor Green
                $foundPaths += $ffmpegDir
            }
        }
    }
}

# Search in common drive roots
Write-Host ""
Write-Host "Surucu kok dizinleri kontrol ediliyor..." -ForegroundColor Yellow
$drives = Get-PSDrive -PSProvider FileSystem | Where-Object { $_.Root -like "?:\" }
foreach ($drive in $drives) {
    $rootPath = $drive.Root + "ffmpeg"
    if (Test-Path $rootPath) {
        if (Test-Path "$rootPath\include\libavcodec\avcodec.h") {
            Write-Host "  [BULUNDU!] $rootPath" -ForegroundColor Green
            $foundPaths += $rootPath
        }
    }
}

Write-Host ""
Write-Host "=== Sonuc ===" -ForegroundColor Cyan
if ($foundPaths.Count -gt 0) {
    Write-Host "[SUCCESS] FFmpeg full kurulumu bulundu!" -ForegroundColor Green
    Write-Host ""
    foreach ($path in $foundPaths) {
        Write-Host "Konum: $path" -ForegroundColor Yellow
        if (Test-Path "$path\include") {
            $includeCount = (Get-ChildItem "$path\include" -Recurse -File -ErrorAction SilentlyContinue).Count
            Write-Host "  Include dosyalari: $includeCount" -ForegroundColor Gray
        }
        if (Test-Path "$path\lib") {
            $libCount = (Get-ChildItem "$path\lib" -Filter "*.lib" -ErrorAction SilentlyContinue).Count
            Write-Host "  Lib dosyalari: $libCount" -ForegroundColor Gray
        }
        Write-Host ""
    }
    
    # Save to file for CMake
    $firstPath = $foundPaths[0]
    Write-Host "CMakeLists.txt'ye eklenecek konum: $firstPath" -ForegroundColor Cyan
    $firstPath | Out-File -FilePath "ffmpeg_path.txt" -Encoding UTF8
    Write-Host "Konum 'ffmpeg_path.txt' dosyasina kaydedildi." -ForegroundColor Green
} else {
    Write-Host "[HATA] FFmpeg full kurulumu bulunamadi!" -ForegroundColor Red
    Write-Host ""
    Write-Host "FFmpeg development dosyalari yuklu degil." -ForegroundColor Yellow
    Write-Host "Lutfen 'fix_ffmpeg.ps1' scriptini calistirin veya manuel olarak yukleyin." -ForegroundColor Yellow
}

Write-Host ""
