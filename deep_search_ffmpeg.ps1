# Deep Search for FFmpeg with include/lib files
Write-Host "FFmpeg full kurulumu derinlemesine araniyor..." -ForegroundColor Cyan
Write-Host ""

# Search for avcodec.h file directly
Write-Host "avcodec.h dosyasi araniyor..." -ForegroundColor Yellow
$avcodecFiles = @()

# Search in common locations
$searchRoots = @(
    "C:\",
    "$env:ProgramFiles",
    "${env:ProgramFiles(x86)}",
    "$env:LOCALAPPDATA",
    "$env:USERPROFILE"
)

foreach ($root in $searchRoots) {
    if (Test-Path $root) {
        Write-Host "Araniyor: $root" -ForegroundColor Gray
        try {
            $files = Get-ChildItem -Path $root -Recurse -Filter "avcodec.h" -ErrorAction SilentlyContinue -Depth 3 | Select-Object -First 5
            foreach ($file in $files) {
                $dir = Split-Path (Split-Path $file.FullName)
                if (Test-Path "$dir\lib\avcodec.lib" -or Test-Path "$dir\lib\libavcodec.a") {
                    Write-Host "  [BULUNDU!] $dir" -ForegroundColor Green
                    $avcodecFiles += $dir
                }
            }
        } catch {
            # Skip if access denied
        }
    }
}

# Also search for libavcodec.lib
Write-Host ""
Write-Host "libavcodec.lib dosyasi araniyor..." -ForegroundColor Yellow
$libFiles = @()

foreach ($root in $searchRoots) {
    if (Test-Path $root) {
        try {
            $files = Get-ChildItem -Path $root -Recurse -Filter "avcodec.lib" -ErrorAction SilentlyContinue -Depth 3 | Select-Object -First 5
            foreach ($file in $files) {
                $libDir = Split-Path $file.FullName
                $rootDir = Split-Path $libDir
                if (Test-Path "$rootDir\include\libavcodec\avcodec.h") {
                    Write-Host "  [BULUNDU!] $rootDir" -ForegroundColor Green
                    $libFiles += $rootDir
                }
            }
        } catch {
            # Skip if access denied
        }
    }
}

# Combine results
$allPaths = ($avcodecFiles + $libFiles) | Select-Object -Unique

Write-Host ""
Write-Host "=== Sonuc ===" -ForegroundColor Cyan
if ($allPaths.Count -gt 0) {
    Write-Host "[SUCCESS] FFmpeg full kurulumu bulundu!" -ForegroundColor Green
    Write-Host ""
    foreach ($path in $allPaths) {
        Write-Host "Konum: $path" -ForegroundColor Yellow
        
        if (Test-Path "$path\include\libavcodec\avcodec.h") {
            Write-Host "  [OK] include/libavcodec/avcodec.h var" -ForegroundColor Green
        }
        
        if (Test-Path "$path\lib") {
            $libs = Get-ChildItem "$path\lib" -Filter "*.lib" -ErrorAction SilentlyContinue
            if ($libs) {
                Write-Host "  [OK] $($libs.Count) .lib dosyasi var" -ForegroundColor Green
            }
        }
        Write-Host ""
    }
    
    # Use first found path
    $ffmpegPath = $allPaths[0]
    Write-Host "CMakeLists.txt'ye eklenecek: $ffmpegPath" -ForegroundColor Cyan
    $ffmpegPath | Out-File -FilePath "ffmpeg_path.txt" -Encoding UTF8
} else {
    Write-Host "[HATA] FFmpeg full kurulumu bulunamadi!" -ForegroundColor Red
    Write-Host ""
    Write-Host "FFmpeg development dosyalari sistemde bulunamadi." -ForegroundColor Yellow
    Write-Host "Lutfen FFmpeg full build'i yukleyin:" -ForegroundColor Yellow
    Write-Host "  1. https://www.gyan.dev/ffmpeg/builds/ adresine gidin" -ForegroundColor White
    Write-Host "  2. 'ffmpeg-release-full.zip' dosyasini indirin" -ForegroundColor White
    Write-Host "  3. C:\ffmpeg dizinine acin" -ForegroundColor White
}

Write-Host ""
