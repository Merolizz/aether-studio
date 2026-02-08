# Git and FetchContent Check Script
Write-Host "Git ve FetchContent Kontrol Ediliyor..." -ForegroundColor Cyan
Write-Host ""

# Check if Git is installed
$gitPath = Get-Command git -ErrorAction SilentlyContinue
if ($gitPath) {
    Write-Host "[OK] Git bulundu:" -ForegroundColor Green
    Write-Host "     $($gitPath.Source)" -ForegroundColor Yellow
    $gitVersion = git --version 2>&1
    Write-Host "     $gitVersion" -ForegroundColor Gray
} else {
    Write-Host "[ERROR] Git bulunamadi!" -ForegroundColor Red
    Write-Host ""
    Write-Host "Git yuklemek icin:" -ForegroundColor Yellow
    Write-Host "  1. https://git-scm.com/download/win adresinden indirin" -ForegroundColor Cyan
    Write-Host "  2. Veya winget ile: winget install Git.Git" -ForegroundColor Cyan
    Write-Host ""
    exit 1
}

Write-Host ""

# Check internet connectivity
Write-Host "Internet baglantisi kontrol ediliyor..." -ForegroundColor Cyan
try {
    $response = Invoke-WebRequest -Uri "https://github.com" -UseBasicParsing -TimeoutSec 5 -ErrorAction Stop
    Write-Host "[OK] GitHub'a erisim mevcut" -ForegroundColor Green
} catch {
    Write-Host "[WARNING] GitHub'a erisilemiyor: $($_.Exception.Message)" -ForegroundColor Yellow
    Write-Host "         FetchContent calismayabilir." -ForegroundColor Yellow
}

Write-Host ""

# Check if CMake cache directories exist (might indicate previous fetch attempts)
$cmakeCacheDir = "$env:USERPROFILE\.cmake"
if (Test-Path $cmakeCacheDir) {
    Write-Host "[INFO] CMake cache dizini bulundu: $cmakeCacheDir" -ForegroundColor Gray
    $fetchDirs = Get-ChildItem -Path $cmakeCacheDir -Directory -Filter "*fetch*" -ErrorAction SilentlyContinue
    if ($fetchDirs) {
        Write-Host "       FetchContent cache dizinleri mevcut" -ForegroundColor Gray
    }
}

Write-Host ""
Write-Host "Eger FetchContent hatalari aliyorsaniz:" -ForegroundColor Yellow
Write-Host "  1. Git'in PATH'te oldugundan emin olun" -ForegroundColor White
Write-Host "  2. Internet baglantinizi kontrol edin" -ForegroundColor White
Write-Host "  3. Visual Studio'yu yonetici olarak calistirmayi deneyin" -ForegroundColor White
Write-Host "  4. CMake cache'i temizleyin (build dizinini silin)" -ForegroundColor White
Write-Host ""
