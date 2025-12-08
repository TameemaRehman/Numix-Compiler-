# PowerShell Run Script for MathSeq Compiler
# This script builds the compiler (release mode) and runs selected examples.

param(
    [string]$Example = "all"
)

$ErrorActionPreference = "Continue"

Write-Host "=====================================" -ForegroundColor Cyan
Write-Host "MathSeq Compiler - Build and Run" -ForegroundColor Cyan
Write-Host "=====================================" -ForegroundColor Cyan
Write-Host ""

# Step 1: Try to build using PowerShell script
Write-Host "Step 1: Building the compiler..." -ForegroundColor Yellow
& powershell -ExecutionPolicy Bypass -File build.ps1 -Mode release

$exampleInputs = @{
    "simple" = @("5")
    "arithmetic" = @("15", "3", "1")
    "fibonacci" = @("10")
    "fibonacci_squared" = @("10")
    "fibonacci_pattern" = @("10", "5", "0")
}

function Get-FilteredExamples {
    param($Example)
    $files = @()
    if (Test-Path "test\examples") {
        $files = Get-ChildItem "test\examples\*.mathseq" -ErrorAction SilentlyContinue
    }
    if ($Example -and $Example.ToLower() -ne "all") {
        $files = $files | Where-Object { $_.BaseName.ToLower() -eq $Example.ToLower() }
    }
    return $files
}

if ($LASTEXITCODE -eq 0) {
    Write-Host ""
    Write-Host "Build successful!" -ForegroundColor Green
    $TARGET = "bin\mathseqc.exe"
    
    if (Test-Path $TARGET) {
        Write-Host ""
        Write-Host "Step 2: Running compiler with examples..." -ForegroundColor Yellow
        Write-Host ""
        
        $testFiles = Get-FilteredExamples -Example $Example
        if ($testFiles.Count -eq 0) {
            Write-Host "No matching test files found in test\examples\" -ForegroundColor Yellow
        } else {
            foreach ($file in $testFiles) {
                Write-Host "=====================================" -ForegroundColor Cyan
                Write-Host "Compiling: $($file.Name)" -ForegroundColor Cyan
                Write-Host "=====================================" -ForegroundColor Cyan
                Write-Host ""
                
                if ($exampleInputs.ContainsKey($file.BaseName)) {
                    $inputData = ($exampleInputs[$file.BaseName] -join "`n") + "`n"
                    $inputData | & $TARGET $file.FullName -ast -output "$($file.BaseName).asm"
                } else {
                    & $TARGET $file.FullName -ast -output "$($file.BaseName).asm"
                }
                Write-Host ""
            }
        }
    }
} else {
    Write-Host ""
    Write-Host "Native compiler not found. Trying WSL..." -ForegroundColor Yellow
    
    try {
        $wslResult = & wsl make release 2>&1
        if ($LASTEXITCODE -eq 0) {
            Write-Host "Build successful with WSL!" -ForegroundColor Green
            $TARGET = "bin\mathseqc"
            
            Write-Host ""
            Write-Host "Step 2: Running compiler with examples..." -ForegroundColor Yellow
            Write-Host ""
            
            $testFiles = Get-FilteredExamples -Example $Example
            if ($testFiles.Count -eq 0) {
                Write-Host "No matching test files found in test\examples\" -ForegroundColor Yellow
            } else {
                foreach ($file in $testFiles) {
                    Write-Host "=====================================" -ForegroundColor Cyan
                    Write-Host "Compiling: $($file.Name)" -ForegroundColor Cyan
                    Write-Host "=====================================" -ForegroundColor Cyan
                    Write-Host ""
                    
                    $wslPath = $file.FullName -replace "C:", "/mnt/c" -replace "\\", "/"
                    $wslOutput = "$($file.BaseName).asm"
                    
                    if ($exampleInputs.ContainsKey($file.BaseName)) {
                        $inputData = ($exampleInputs[$file.BaseName] -join "`n") + "`n"
                        $tempFile = New-TemporaryFile
                        Set-Content -Path $tempFile -Value $inputData -Encoding UTF8
                        try {
                            Get-Content $tempFile | & wsl $TARGET $wslPath -ast -output $wslOutput
                        } finally {
                            Remove-Item $tempFile -ErrorAction SilentlyContinue
                        }
                    } else {
                        & wsl $TARGET $wslPath -ast -output $wslOutput
                    }
                    Write-Host ""
                }
            }
        } else {
            Write-Host ""
            Write-Host "Build failed. Error message:" -ForegroundColor Red
            Write-Host $wslResult -ForegroundColor Red
            Write-Host ""
            Write-Host "Please ensure:" -ForegroundColor Yellow
            Write-Host "  1. WSL is installed and Ubuntu is set up" -ForegroundColor Yellow
            Write-Host "  2. You have run: wsl (to initialize Ubuntu)" -ForegroundColor Yellow
            Write-Host "  3. Build tools are installed in WSL: sudo apt update && sudo apt install -y build-essential" -ForegroundColor Yellow
            Write-Host ""
            Write-Host "Or install a native C++ compiler:" -ForegroundColor Yellow
            Write-Host "  - MinGW-w64: https://www.mingw-w64.org/downloads/" -ForegroundColor Yellow
            Write-Host "  - Visual Studio Build Tools: https://visualstudio.microsoft.com/downloads/" -ForegroundColor Yellow
            exit 1
        }
    } catch {
        Write-Host ""
        Write-Host "WSL build failed: $_" -ForegroundColor Red
        Write-Host ""
        Write-Host "Options:" -ForegroundColor Yellow
        Write-Host "  1. Install WSL and Ubuntu: wsl --install -d Ubuntu" -ForegroundColor Yellow
        Write-Host "  2. After reboot, run: wsl (to initialize)" -ForegroundColor Yellow
        Write-Host "  3. Install build tools in WSL: sudo apt update && sudo apt install -y build-essential" -ForegroundColor Yellow
        Write-Host "  4. Then run: wsl make release" -ForegroundColor Yellow
        exit 1
    }
}

Write-Host "=====================================" -ForegroundColor Green
Write-Host "Done! Compiler built and tested." -ForegroundColor Green
Write-Host "=====================================" -ForegroundColor Green

