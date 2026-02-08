# Clean Build Cache Script
Write-Host "Build cache temizleniyor..." -ForegroundColor Cyan
Write-Host ""

$buildDirs = @(
    "out\build",
    "build",
    "out\build\x64-Debug\_deps",
    "build\_deps"
)

# Also clean specific FetchContent directories
$fetchDirs = @(
    "build\_deps\imgui-subbuild",
    "build\_deps\glfw-subbuild",
    "build\_deps\json-subbuild",
    "out\build\x64-Debug\_deps\imgui-subbuild",
    "out\build\x64-Debug\_deps\glfw-subbuild",
    "out\build\x64-Debug\_deps\json-subbuild"
)

$cleaned = $false

foreach ($dir in $buildDirs) {
    if (Test-Path $dir) {
        Write-Host "Siliniyor: $dir" -ForegroundColor Yellow
        try {
            Remove-Item -Path $dir -Recurse -Force -ErrorAction Stop
            Write-Host "  [OK] Temizlendi" -ForegroundColor Green
            $cleaned = $true
        } catch {
            Write-Host "  [HATA] Silinemedi: $($_.Exception.Message)" -ForegroundColor Red
        }
    } else {
        Write-Host "Bulunamadi: $dir" -ForegroundColor Gray
    }
}

# Clean FetchContent subbuild directories
Write-Host ""
Write-Host "FetchContent subbuild dizinleri temizleniyor..." -ForegroundColor Cyan
foreach ($dir in $fetchDirs) {
    if (Test-Path $dir) {
        Write-Host "Siliniyor: $dir" -ForegroundColor Yellow
        try {
            Remove-Item -Path $dir -Recurse -Force -ErrorAction Stop
            Write-Host "  [OK] Temizlendi" -ForegroundColor Green
            $cleaned = $true
        } catch {
            Write-Host "  [HATA] Silinemedi: $($_.Exception.Message)" -ForegroundColor Red
        }
    }
}

# Also clean CMake cache in user directory
$cmakeCache = "$env:USERPROFILE\.cmake\packages"
if (Test-Path $cmakeCache) {
    Write-Host ""
    Write-Host "CMake paket cache temizleniyor..." -ForegroundColor Cyan
    try {
        Get-ChildItem -Path $cmakeCache -Directory -Filter "*imgui*" -ErrorAction SilentlyContinue | 
            Remove-Item -Recurse -Force -ErrorAction SilentlyContinue
        Get-ChildItem -Path $cmakeCache -Directory -Filter "*glfw*" -ErrorAction SilentlyContinue | 
            Remove-Item -Recurse -Force -ErrorAction SilentlyContinue
        Get-ChildItem -Path $cmakeCache -Directory -Filter "*json*" -ErrorAction SilentlyContinue | 
            Remove-Item -Recurse -Force -ErrorAction SilentlyContinue
        Write-Host "  [OK] CMake cache temizlendi" -ForegroundColor Green
        $cleaned = $true
    } catch {
        Write-Host "  [UYARI] CMake cache temizlenemedi: $($_.Exception.Message)" -ForegroundColor Yellow
    }
}

Write-Host ""
if ($cleaned) {
    Write-Host "[SUCCESS] Build cache temizlendi!" -ForegroundColor Green
    Write-Host "Visual Studio'da CMake'i yeniden yapilandirin." -ForegroundColor Yellow
} else {
    Write-Host "[INFO] Temizlenecek cache bulunamadi." -ForegroundColor Gray
}

Write-Host ""
