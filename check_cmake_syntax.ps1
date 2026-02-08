# Check CMake syntax more accurately
$file = "CMakeLists.txt"
$lines = Get-Content $file

$ifStack = @()
$errors = @()
$lineNum = 0

foreach ($line in $lines) {
    $lineNum++
    $trimmed = $line.Trim()
    
    if ($trimmed -match '^\s*if\s*\(') {
        $ifStack += $lineNum
    }
    elseif ($trimmed -match '^\s*endif\s*\(\s*\)') {
        if ($ifStack.Count -eq 0) {
            $errors += "Line $lineNum : endif() without matching if()"
        } else {
            $ifStack = $ifStack[0..($ifStack.Count-2)]
        }
    }
    elseif ($trimmed -match '^\s*elseif\s*\(') {
        # elseif doesn't change stack
    }
    elseif ($trimmed -match '^\s*else\s*\(\s*\)') {
        # else doesn't change stack
    }
}

Write-Host "CMakeLists.txt syntax kontrolu" -ForegroundColor Cyan
Write-Host ""

if ($ifStack.Count -eq 0 -and $errors.Count -eq 0) {
    Write-Host "[OK] if/endif eslesiyor" -ForegroundColor Green
} else {
    if ($ifStack.Count -gt 0) {
        Write-Host "[HATA] $($ifStack.Count) acik if() blogu var (endif eksik):" -ForegroundColor Red
        foreach ($line in $ifStack) {
            Write-Host "  Line $line : if() acik kaldi" -ForegroundColor Yellow
        }
    }
    if ($errors.Count -gt 0) {
        foreach ($err in $errors) {
            Write-Host "[HATA] $err" -ForegroundColor Red
        }
    }
}

Write-Host ""
