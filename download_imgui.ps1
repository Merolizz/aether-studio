# ImGui Download and Install Script
Write-Host "ImGui indiriliyor ve yukleniyor..." -ForegroundColor Cyan
Write-Host ""

$projectRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
$imguiDir = Join-Path $projectRoot "third_party\imgui"

# Create third_party directory if it doesn't exist
$thirdPartyDir = Join-Path $projectRoot "third_party"
if (-not (Test-Path $thirdPartyDir)) {
    New-Item -ItemType Directory -Path $thirdPartyDir -Force | Out-Null
    Write-Host "third_party dizini olusturuldu" -ForegroundColor Green
}

# Remove old imgui if exists
if (Test-Path $imguiDir) {
    Write-Host "Eski ImGui dizini temizleniyor..." -ForegroundColor Yellow
    Remove-Item -Path $imguiDir -Recurse -Force -ErrorAction SilentlyContinue
}

# Download ImGui - try multiple methods
$urls = @(
    "https://github.com/ocornut/imgui/archive/refs/heads/master.zip",
    "https://github.com/ocornut/imgui/archive/v1.90.0.zip",
    "https://github.com/ocornut/imgui/archive/refs/tags/v1.90.0.zip"
)

$zipFile = Join-Path $env:TEMP "imgui.zip"
$extractDir = Join-Path $env:TEMP "imgui-extract"

Write-Host "ImGui indiriliyor..." -ForegroundColor Yellow
$success = $false

foreach ($url in $urls) {
    try {
        Write-Host "Deneniyor: $url" -ForegroundColor Gray
        Invoke-WebRequest -Uri $url -OutFile $zipFile -UseBasicParsing -ErrorAction Stop
        $success = $true
        Write-Host "[OK] Indirme tamamlandi" -ForegroundColor Green
        break
    } catch {
        Write-Host "  Basarisiz: $($_.Exception.Message)" -ForegroundColor Yellow
        continue
    }
}

if ($success) {
try {
    Write-Host "[OK] Indirme tamamlandi" -ForegroundColor Green
    
    Write-Host "Aciliyor..." -ForegroundColor Yellow
    if (Test-Path $extractDir) {
        Remove-Item -Path $extractDir -Recurse -Force
    }
    Expand-Archive -Path $zipFile -DestinationPath $extractDir -Force
    
    # Find the extracted folder
    $extractedFolder = Get-ChildItem -Path $extractDir -Directory | Where-Object { $_.Name -like "imgui-*" } | Select-Object -First 1
    
    if ($extractedFolder) {
        Write-Host "Kopyalaniyor..." -ForegroundColor Yellow
        Move-Item -Path $extractedFolder.FullName -Destination $imguiDir -Force
        Write-Host "[OK] ImGui yuklendi: $imguiDir" -ForegroundColor Green
        
        # Verify installation
        $imguiCpp = Join-Path $imguiDir "imgui.cpp"
        if (Test-Path $imguiCpp) {
            Write-Host "[SUCCESS] ImGui basariyla yuklendi!" -ForegroundColor Green
            Write-Host "Konum: $imguiDir" -ForegroundColor Cyan
        } else {
            Write-Host "[HATA] ImGui dosyalari bulunamadi!" -ForegroundColor Red
        }
    } else {
        Write-Host "[HATA] Acilan dizin bulunamadi!" -ForegroundColor Red
    }
    
    # Cleanup
    Remove-Item -Path $zipFile -Force -ErrorAction SilentlyContinue
    Remove-Item -Path $extractDir -Recurse -Force -ErrorAction SilentlyContinue
    
} catch {
    Write-Host "[HATA] Acilma/kopyalama basarisiz: $($_.Exception.Message)" -ForegroundColor Red
    exit 1
}
} else {
    Write-Host "[HATA] Tum indirme yontemleri basarisiz!" -ForegroundColor Red
    exit 1
}

Write-Host ""
