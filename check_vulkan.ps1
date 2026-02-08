# Vulkan SDK Check and Setup Script
Write-Host "Vulkan SDK Kontrol Ediliyor..." -ForegroundColor Cyan
Write-Host ""

# Check environment variable
if ($env:VULKAN_SDK) {
    Write-Host "[OK] VULKAN_SDK environment variable is set:" -ForegroundColor Green
    Write-Host "     $env:VULKAN_SDK" -ForegroundColor Yellow
    if (Test-Path $env:VULKAN_SDK) {
        Write-Host "[OK] Path exists" -ForegroundColor Green
    } else {
        Write-Host "[ERROR] Path does not exist!" -ForegroundColor Red
    }
} else {
    Write-Host "[WARNING] VULKAN_SDK environment variable is not set" -ForegroundColor Yellow
}

Write-Host ""

# Check common installation paths
$searchPaths = @(
    "C:\VulkanSDK",
    "$env:ProgramFiles\VulkanSDK",
    "${env:ProgramFiles(x86)}\VulkanSDK"
)

$foundSdk = $null

foreach ($basePath in $searchPaths) {
    if (Test-Path $basePath) {
        Write-Host "Found VulkanSDK directory: $basePath" -ForegroundColor Green
        
        # Look for versioned subdirectories
        $versionDirs = Get-ChildItem -Path $basePath -Directory -ErrorAction SilentlyContinue | 
                       Where-Object { $_.Name -match '^\d+\.\d+' } |
                       Sort-Object Name -Descending
        
        if ($versionDirs) {
            foreach ($versionDir in $versionDirs) {
                $fullPath = $versionDir.FullName
                $vulkanH = Join-Path $fullPath "Include\vulkan\vulkan.h"
                
                if (Test-Path $vulkanH) {
                    Write-Host "  -> Found SDK version: $($versionDir.Name)" -ForegroundColor Cyan
                    Write-Host "     Path: $fullPath" -ForegroundColor Gray
                    
                    if (-not $foundSdk) {
                        $foundSdk = $fullPath
                    }
                }
            }
        } else {
            # Check if base path itself contains the SDK
            $vulkanH = Join-Path $basePath "Include\vulkan\vulkan.h"
            if (Test-Path $vulkanH) {
                Write-Host "  -> Found SDK at base path" -ForegroundColor Cyan
                if (-not $foundSdk) {
                    $foundSdk = $basePath
                }
            }
        }
    }
}

Write-Host ""

if ($foundSdk) {
    Write-Host "[SUCCESS] Vulkan SDK found at: $foundSdk" -ForegroundColor Green
    
    if (-not $env:VULKAN_SDK -or $env:VULKAN_SDK -ne $foundSdk) {
        Write-Host ""
        Write-Host "To set the environment variable, run:" -ForegroundColor Yellow
        Write-Host "  [System.Environment]::SetEnvironmentVariable('VULKAN_SDK', '$foundSdk', 'User')" -ForegroundColor White
        Write-Host ""
        $response = Read-Host "Set VULKAN_SDK environment variable now? (Y/N)"
        if ($response -eq 'Y' -or $response -eq 'y') {
            [System.Environment]::SetEnvironmentVariable('VULKAN_SDK', $foundSdk, 'User')
            $env:VULKAN_SDK = $foundSdk
            Write-Host "[OK] Environment variable set!" -ForegroundColor Green
            Write-Host "     Please restart Visual Studio for changes to take effect." -ForegroundColor Yellow
        }
    } else {
        Write-Host "[OK] Environment variable is already set correctly" -ForegroundColor Green
    }
} else {
    Write-Host "[ERROR] Vulkan SDK not found!" -ForegroundColor Red
    Write-Host ""
    Write-Host "Please install Vulkan SDK from:" -ForegroundColor Yellow
    Write-Host "  https://vulkan.lunarg.com/sdk/home" -ForegroundColor Cyan
    Write-Host ""
    Write-Host "Or run the install_vulkan.ps1 script to download and install it." -ForegroundColor Yellow
}

Write-Host ""
