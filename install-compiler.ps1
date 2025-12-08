# Quick Compiler Installation Helper for Windows
# This script helps you install a C++ compiler without admin rights

Write-Host "C++ Compiler Installation Helper" -ForegroundColor Cyan
Write-Host "================================" -ForegroundColor Cyan
Write-Host ""

# Check if we have admin rights
$isAdmin = ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)

if ($isAdmin) {
    Write-Host "Running with administrator rights!" -ForegroundColor Green
    Write-Host ""
    
    Write-Host "Option 1: Install via Chocolatey (recommended with admin)" -ForegroundColor Yellow
    Write-Host "  Run: choco install mingw -y" -ForegroundColor White
    Write-Host ""
    
    Write-Host "Option 2: Install winlibs (portable, no installation needed)" -ForegroundColor Yellow
    Write-Host "  Run: choco install winlibs -y" -ForegroundColor White
    Write-Host ""
    
    Write-Host "Option 3: Install MSYS2" -ForegroundColor Yellow
    Write-Host "  Run: choco install msys2 -y" -ForegroundColor White
    Write-Host ""
    
} else {
    Write-Host "No administrator rights detected." -ForegroundColor Yellow
    Write-Host "Recommended options for non-admin installation:" -ForegroundColor Cyan
    Write-Host ""
    
    Write-Host "Option 1: MSYS2 (Best for non-admin)" -ForegroundColor Green
    Write-Host "  1. Download from: https://www.msys2.org/" -ForegroundColor White
    Write-Host "  2. Install to your user directory (no admin needed)" -ForegroundColor White
    Write-Host "  3. Open MSYS2 terminal and run:" -ForegroundColor White
    Write-Host "     pacman -Syu" -ForegroundColor Gray
    Write-Host "     pacman -S mingw-w64-x86_64-gcc" -ForegroundColor Gray
    Write-Host "  4. Add C:\msys64\mingw64\bin to your PATH" -ForegroundColor White
    Write-Host ""
    
    Write-Host "Option 2: WinLibs (Portable, no installation)" -ForegroundColor Green
    Write-Host "  1. Download from: https://winlibs.com/" -ForegroundColor White
    Write-Host "  2. Extract to a folder (e.g., C:\mingw64)" -ForegroundColor White
    Write-Host "  3. Add the \bin folder to your PATH" -ForegroundColor White
    Write-Host ""
    
    Write-Host "Option 3: Try Chocolatey (may work without admin)" -ForegroundColor Green
    Write-Host "  Note: You may need to clean lock files first" -ForegroundColor Yellow
    Write-Host "  Run: choco install winlibs -y" -ForegroundColor White
    Write-Host ""
}

Write-Host "Current compiler status:" -ForegroundColor Cyan
$compilers = @("g++", "gcc", "clang++", "cl")
$found = $false

foreach ($comp in $compilers) {
    $cmd = Get-Command $comp -ErrorAction SilentlyContinue
    if ($cmd) {
        Write-Host "  [OK] Found: $comp at $($cmd.Source)" -ForegroundColor Green
        $found = $true
    }
}

if (-not $found) {
    Write-Host "  [X] No C++ compiler found" -ForegroundColor Red
    Write-Host ""
    Write-Host "See INSTALL_COMPILER.md for detailed installation instructions." -ForegroundColor Yellow
} else {
    Write-Host ""
    Write-Host "You're all set! Try running: .\make release" -ForegroundColor Green
}

