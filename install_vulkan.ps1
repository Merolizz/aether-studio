# Vulkan SDK Download and Install Script
Write-Host "Vulkan SDK Indiriliyor..." -ForegroundColor Green

$url = "https://sdk.lunarg.com/sdk/download/latest/windows/vulkan_sdk.exe"
$installer = "$env:TEMP\vulkan_sdk.exe"

try {
    # Download
    Write-Host "Indirme basladi..." -ForegroundColor Yellow
    Invoke-WebRequest -Uri $url -OutFile $installer -UseBasicParsing
    
    $fileInfo = Get-Item $installer
    Write-Host "Indirme tamamlandi!" -ForegroundColor Green
    Write-Host "Dosya boyutu: $([math]::Round($fileInfo.Length/1MB, 2)) MB" -ForegroundColor Cyan
    Write-Host "Konum: $installer" -ForegroundColor Cyan
    
    # Install
    Write-Host "`nKurulum baslatiliyor..." -ForegroundColor Yellow
    Write-Host "Lutfen kurulum penceresinde 'Next' butonlarina tiklayarak kurulumu tamamlayin." -ForegroundColor Yellow
    
    Start-Process -FilePath $installer -Wait
    
    Write-Host "`nKurulum tamamlandi!" -ForegroundColor Green
    Write-Host "Visual Studio'yu yeniden baslatin ve projeyi tekrar derleyin." -ForegroundColor Yellow
    
} catch {
    Write-Host "Hata: $($_.Exception.Message)" -ForegroundColor Red
    Write-Host "`nManuel indirme icin: https://vulkan.lunarg.com/sdk/home" -ForegroundColor Yellow
}
