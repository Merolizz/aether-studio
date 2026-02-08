# Validate CMakeLists.txt syntax
Write-Host "CMakeLists.txt syntax kontrol ediliyor..." -ForegroundColor Cyan
Write-Host ""

$file = "CMakeLists.txt"
$content = Get-Content $file -Raw

# Count if/endif
$ifMatches = [regex]::Matches($content, '\bif\s*\(')
$endifMatches = [regex]::Matches($content, '\bendif\s*\(\s*\)')
$elseifMatches = [regex]::Matches($content, '\belseif\s*\(')
$elseMatches = [regex]::Matches($content, '\belse\s*\(\s*\)')

Write-Host "if: $($ifMatches.Count)" -ForegroundColor Yellow
Write-Host "endif: $($endifMatches.Count)" -ForegroundColor Yellow
Write-Host "elseif: $($elseifMatches.Count)" -ForegroundColor Yellow
Write-Host "else: $($elseMatches.Count)" -ForegroundColor Yellow
Write-Host ""

$totalIf = $ifMatches.Count + $elseifMatches.Count
$totalEnd = $endifMatches.Count

if ($totalIf -eq $totalEnd) {
    Write-Host "[OK] if/endif eslesiyor" -ForegroundColor Green
} else {
    Write-Host "[HATA] if/endif eslesmiyor!" -ForegroundColor Red
    Write-Host "  if/elseif toplam: $totalIf" -ForegroundColor Yellow
    Write-Host "  endif toplam: $totalEnd" -ForegroundColor Yellow
    Write-Host "  Fark: $($totalIf - $totalEnd)" -ForegroundColor Red
}

Write-Host ""
