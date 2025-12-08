# PowerShell Build Script for MathSeq Compiler

param(
    [string]$Mode = "release",
    [switch]$Test = $false,
    [switch]$Clean = $false
)

$ErrorActionPreference = "Stop"

# Directories
$SRCDIR = "src"
$INCDIR = "include"
$BUILDDIR = "build"
$TARGETDIR = "bin"
$TARGET = "$TARGETDIR\mathseqc.exe"

# Source files
$SOURCES = @(
    "$SRCDIR\main.cpp",
    "$SRCDIR\lexer.cpp",
    "$SRCDIR\parser.cpp",
    "$SRCDIR\semantic.cpp",
    "$SRCDIR\symbol_table.cpp",
    "$SRCDIR\codegen.cpp",
    "$SRCDIR\optimizer.cpp",
    "$SRCDIR\interpreter.cpp"
)

# Try to find a C++ compiler
$CXX = $null
$CXXFLAGS = "-std=c++17 -Wall -Wextra -I$INCDIR"

if ($Mode -eq "debug") {
    $CXXFLAGS += " -g -DDEBUG"
} else {
    $CXXFLAGS += " -O3"
}

# Check for g++ (MinGW)
try {
    $null = & g++ --version 2>$null
    $CXX = "g++"
    Write-Host "Found g++ compiler" -ForegroundColor Green
} catch {
    # Try clang++
    try {
        $null = & clang++ --version 2>$null
        $CXX = "clang++"
        Write-Host "Found clang++ compiler" -ForegroundColor Green
    } catch {
        # Try MSVC cl
        try {
            $null = & cl 2>$null
            $CXX = "cl"
            $CXXFLAGS = "/std:c++17 /EHsc /I$INCDIR"
            if ($Mode -eq "debug") {
                $CXXFLAGS += " /Zi /DDEBUG"
            } else {
                $CXXFLAGS += " /O2"
            }
            Write-Host "Found MSVC compiler" -ForegroundColor Green
        } catch {
            Write-Host "ERROR: No C++ compiler found!" -ForegroundColor Red
            Write-Host ""
            Write-Host "Please install one of the following:" -ForegroundColor Yellow
            Write-Host "  1. MinGW-w64 (for g++)" -ForegroundColor Yellow
            Write-Host "  2. Clang (for clang++)" -ForegroundColor Yellow
            Write-Host "  3. Visual Studio Build Tools (for MSVC cl)" -ForegroundColor Yellow
            Write-Host ""
            Write-Host "Or use WSL with:" -ForegroundColor Yellow
            Write-Host "  wsl --install -d Ubuntu" -ForegroundColor Yellow
            Write-Host "  wsl make release" -ForegroundColor Yellow
            exit 1
        }
    }
}

# Clean build
if ($Clean) {
    Write-Host "Cleaning build directories..." -ForegroundColor Yellow
    if (Test-Path $BUILDDIR) { Remove-Item -Recurse -Force $BUILDDIR }
    if (Test-Path $TARGETDIR) { Remove-Item -Recurse -Force $TARGETDIR }
    Write-Host "Clean complete!" -ForegroundColor Green
    exit 0
}

# Create directories
if (-not (Test-Path $BUILDDIR)) {
    New-Item -ItemType Directory -Path $BUILDDIR | Out-Null
}
if (-not (Test-Path $TARGETDIR)) {
    New-Item -ItemType Directory -Path $TARGETDIR | Out-Null
}

Write-Host "Building MathSeq Compiler ($Mode mode)..." -ForegroundColor Cyan
Write-Host "Using compiler: $CXX" -ForegroundColor Cyan
Write-Host ""

# Compile object files
$OBJECTS = @()
foreach ($source in $SOURCES) {
    $objFile = $source -replace "$SRCDIR\\(.*)\.cpp", "$BUILDDIR\`$1.o"
    $objFile = $objFile -replace "\\", "/"
    
    Write-Host "Compiling: $source" -ForegroundColor Gray
    
    if ($CXX -eq "cl") {
        # MSVC compilation
        $objFile = $objFile -replace "\.o$", ".obj"
        & $CXX $CXXFLAGS.Split(" ") /c $source /Fo"$objFile" /nologo
        if ($LASTEXITCODE -ne 0) {
            Write-Host "Compilation failed!" -ForegroundColor Red
            exit 1
        }
    } else {
        # g++/clang++ compilation
        & $CXX $CXXFLAGS.Split(" ") -c $source -o $objFile
        if ($LASTEXITCODE -ne 0) {
            Write-Host "Compilation failed!" -ForegroundColor Red
            exit 1
        }
    }
    
    $OBJECTS += $objFile
}

# Link executable
Write-Host ""
Write-Host "Linking executable..." -ForegroundColor Cyan

if ($CXX -eq "cl") {
    & $CXX $OBJECTS /Fe"$TARGET" /nologo
} else {
    & $CXX $CXXFLAGS.Split(" ") $OBJECTS -o $TARGET
}

if ($LASTEXITCODE -ne 0) {
    Write-Host "Linking failed!" -ForegroundColor Red
    exit 1
}

Write-Host ""
Write-Host "Build complete: $TARGET" -ForegroundColor Green

# Run tests if requested
if ($Test) {
    Write-Host ""
    Write-Host "Running tests..." -ForegroundColor Cyan
    if (Test-Path "test\examples\fibonacci.mathseq") {
        & $TARGET "test\examples\fibonacci.mathseq" -ast
    } else {
        Write-Host "Test files not found!" -ForegroundColor Yellow
    }
}

