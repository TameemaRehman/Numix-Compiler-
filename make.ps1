# PowerShell replacement for Makefile
# Usage: .\make.ps1 [target]
# Targets: clean, debug, release, test, example, help

param(
    [Parameter(Position=0)]
    [string]$Target = "help",
    
    [Parameter(ValueFromRemainingArguments=$true)]
    [string[]]$ExtraArgs = @()
)

# Project directory (adjust if needed)
$ProjectDir = "MathScript-Compiler-main"

# Compiler settings - will auto-detect if available
$CXX = $null
$CXXFlags = ""
$DebugFlags = ""
$ReleaseFlags = ""

# Try to auto-detect compiler
function Find-Compiler {
    $compilers = @(
        @{Name="g++"; Flags="-std=c++17 -Wall -Wextra"; Debug="-g -DDEBUG"; Release="-O3"},
        @{Name="gcc"; Flags="-std=c++17 -Wall -Wextra"; Debug="-g -DDEBUG"; Release="-O3"},
        @{Name="clang++"; Flags="-std=c++17 -Wall -Wextra"; Debug="-g -DDEBUG"; Release="-O3"},
        @{Name="cl"; Flags="/std:c++17 /EHsc /W3"; Debug="/Zi /DEBUG /DDEBUG"; Release="/O2"}
    )
    
    foreach ($comp in $compilers) {
        try {
            $null = Get-Command $comp.Name -ErrorAction Stop
            Write-Host "Found compiler: $($comp.Name)" -ForegroundColor Green
            return $comp
        } catch {
            continue
        }
    }
    
    return $null
}

$DetectedCompiler = Find-Compiler
if ($DetectedCompiler) {
    $CXX = $DetectedCompiler.Name
    $CXXFlags = $DetectedCompiler.Flags
    $DebugFlags = $DetectedCompiler.Debug
    $ReleaseFlags = $DetectedCompiler.Release
} else {
    # Will be checked in Build-Project function
}

# Directories
$SrcDir = Join-Path $ProjectDir "src"
$IncDir = Join-Path $ProjectDir "include"
$BuildDir = Join-Path $ProjectDir "build"
$BinDir = Join-Path $ProjectDir "bin"
$TargetExe = Join-Path $BinDir "mathseqc.exe"

function Clean-Build {
    Write-Host "Cleaning build artifacts..." -ForegroundColor Yellow
    
    if (Test-Path $BuildDir) {
        Remove-Item -Path $BuildDir -Recurse -Force
        Write-Host "Removed $BuildDir directory" -ForegroundColor Green
    }
    
    if (Test-Path $BinDir) {
        Remove-Item -Path $BinDir -Recurse -Force
        Write-Host "Removed $BinDir directory" -ForegroundColor Green
    }
    
    Write-Host "Clean complete!" -ForegroundColor Green
}

function Build-Debug {
    Write-Host "Building debug version..." -ForegroundColor Yellow
    Build-Project -Mode "debug"
}

function Build-Release {
    Write-Host "Building release version..." -ForegroundColor Yellow
    Build-Project -Mode "release"
}

function Build-Project {
    param([string]$Mode)
    
    # Check if compiler is available
    if (-not $CXX) {
        Write-Host "Error: No C++ compiler found!" -ForegroundColor Red
        Write-Host ""
        Write-Host "Please install one of the following:" -ForegroundColor Yellow
        Write-Host "  1. MinGW-w64 (recommended):" -ForegroundColor Cyan
        Write-Host "     - Download from: https://www.mingw-w64.org/downloads/" -ForegroundColor White
        Write-Host "     - Or use MSYS2: https://www.msys2.org/" -ForegroundColor White
        Write-Host "     - Or install via Chocolatey: choco install mingw" -ForegroundColor White
        Write-Host ""
        Write-Host "  2. Microsoft Visual Studio (with C++ tools):" -ForegroundColor Cyan
        Write-Host "     - Download from: https://visualstudio.microsoft.com/" -ForegroundColor White
        Write-Host "     - Make sure to install 'Desktop development with C++' workload" -ForegroundColor White
        Write-Host ""
        Write-Host "  3. LLVM/Clang:" -ForegroundColor Cyan
        Write-Host "     - Download from: https://llvm.org/builds/" -ForegroundColor White
        Write-Host ""
        Write-Host "After installation, restart PowerShell and try again." -ForegroundColor Yellow
        Write-Host ""
        Write-Host "Alternatively, you can edit make.ps1 and set `$CXX to your compiler path." -ForegroundColor Yellow
        exit 1
    }
    
    try {
        $null = Get-Command $CXX -ErrorAction Stop
    } catch {
        Write-Host "Error: Compiler '$CXX' not found in PATH." -ForegroundColor Red
        Write-Host "Please make sure your compiler is installed and added to PATH." -ForegroundColor Yellow
        exit 1
    }
    
    # Create directories
    if (-not (Test-Path $BuildDir)) {
        New-Item -ItemType Directory -Path $BuildDir | Out-Null
    }
    
    if (-not (Test-Path $BinDir)) {
        New-Item -ItemType Directory -Path $BinDir | Out-Null
    }
    
    # Get source files
    $SourceFiles = Get-ChildItem -Path $SrcDir -Filter "*.cpp"
    
    if ($SourceFiles.Count -eq 0) {
        Write-Host "Error: No source files found in $SrcDir" -ForegroundColor Red
        exit 1
    }
    
    # Set flags based on mode
    $CompileFlags = $CXXFlags
    if ($Mode -eq "debug") {
        $CompileFlags += " $DebugFlags"
    } else {
        $CompileFlags += " $ReleaseFlags"
    }
    
    # Determine file extensions and compile flags based on compiler
    $IsMSVC = ($CXX -eq "cl")
    $ObjExt = if ($IsMSVC) { ".obj" } else { ".o" }
    $IncludeFlag = if ($IsMSVC) { "/I" } else { "-I" }
    
    # Add include directory to flags
    $FullIncludeDir = Join-Path (Resolve-Path $ProjectDir) "include"
    $CompileFlags = "$CompileFlags $IncludeFlag`"$FullIncludeDir`""
    
    # Compile source files
    $ObjectFiles = @()
    $Failed = $false
    
    foreach ($Source in $SourceFiles) {
        $ObjectFile = Join-Path $BuildDir "$($Source.BaseName)$ObjExt"
        $ObjectFiles += $ObjectFile
        
        Write-Host "Compiling $($Source.Name)..." -ForegroundColor Cyan
        
        if ($IsMSVC) {
            # MSVC uses different syntax
            $CompileCmd = "$CXX $CompileFlags /c `"$($Source.FullName)`" /Fo:`"$ObjectFile`""
        } else {
            # GCC/Clang syntax
            $CompileCmd = "$CXX $CompileFlags -c `"$($Source.FullName)`" -o `"$ObjectFile`""
        }
        
        $result = & cmd /c "$CompileCmd 2>&1"
        $exitCode = $LASTEXITCODE
        
        if ($exitCode -ne 0) {
            Write-Host "Compilation failed for $($Source.Name):" -ForegroundColor Red
            Write-Host $result -ForegroundColor Red
            Write-Host "Command was: $CompileCmd" -ForegroundColor Yellow
            $Failed = $true
            break
        }
        
        # Show any warnings
        if ($result -and ($result -match "warning" -or $result -match "Warning")) {
            Write-Host $result -ForegroundColor Yellow
        }
    }
    
    if ($Failed) {
        Write-Host "Build failed!" -ForegroundColor Red
        exit 1
    }
    
    # Link
    Write-Host "Linking..." -ForegroundColor Cyan
    $ObjectFilesString = ($ObjectFiles | ForEach-Object { "`"$_`"" }) -join " "
    
    if ($IsMSVC) {
        # MSVC linking
        $LinkCmd = "$CXX $CompileFlags $ObjectFilesString /Fe:`"$TargetExe`""
    } else {
        # GCC/Clang linking
        $LinkCmd = "$CXX $CompileFlags $ObjectFilesString -o `"$TargetExe`""
    }
    
    $result = & cmd /c "$LinkCmd 2>&1"
    $exitCode = $LASTEXITCODE
    
    if ($exitCode -ne 0) {
        Write-Host "Linking failed:" -ForegroundColor Red
        Write-Host $result -ForegroundColor Red
        Write-Host "Command was: $LinkCmd" -ForegroundColor Yellow
        exit 1
    }
    
    Write-Host "Build complete: $TargetExe" -ForegroundColor Green
}

function Run-Test {
    Write-Host "Running tests..." -ForegroundColor Yellow
    
    # Build debug first
    Build-Debug
    
    if (-not (Test-Path $TargetExe)) {
        Write-Host "Error: Executable not found!" -ForegroundColor Red
        exit 1
    }
    
    $TestFile = Join-Path $ProjectDir "test\examples\fibonacci.mathseq"
    Write-Host "Running: $TargetExe $TestFile -ast" -ForegroundColor Cyan
    
    & $TargetExe $TestFile -ast
}

function Run-Example {
    Write-Host "Running example..." -ForegroundColor Yellow
    
    # Build debug first
    Build-Debug
    
    if (-not (Test-Path $TargetExe)) {
        Write-Host "Error: Executable not found!" -ForegroundColor Red
        exit 1
    }
    
    $TestFile = Join-Path $ProjectDir "test\examples\fibonacci.mathseq"
    $OutputFile = Join-Path $ProjectDir "output.asm"
    Write-Host "Running: $TargetExe $TestFile -output $OutputFile" -ForegroundColor Cyan
    
    & $TargetExe $TestFile -output $OutputFile
}

function Run-Compiler {
    param([Parameter(ValueFromRemainingArguments=$true)][string[]]$args)
    
    if (-not (Test-Path $TargetExe)) {
        Write-Host "Error: Compiler not found. Building first..." -ForegroundColor Yellow
        Build-Release
    }
    
    if ($args.Count -eq 0) {
        Write-Host "Usage: .\make run [source-file] [options]" -ForegroundColor Yellow
        Write-Host "Example: .\make run MathScript-Compiler-main\test\examples\simple.mathseq -ast" -ForegroundColor White
        exit 1
    }
    
    $filePath = $args[0]
    $remainingArgs = $args[1..($args.Length-1)]
    
    # Convert forward slashes to backslashes
    $filePath = $filePath -replace '/', '\'
    
    # Try to resolve relative paths
    if (-not [System.IO.Path]::IsPathRooted($filePath)) {
        $testPath = Join-Path $ProjectDir $filePath
        if (Test-Path $testPath) {
            $filePath = $testPath
        } elseif (-not (Test-Path $filePath)) {
            Write-Host "Error: Could not find file: $filePath" -ForegroundColor Red
            exit 1
        }
    }
    
    $cmd = "& `"$TargetExe`" `"$filePath`""
    if ($remainingArgs.Count -gt 0) {
        $cmd += " $remainingArgs"
    }
    
    Invoke-Expression $cmd
}

function Show-Help {
    Write-Host "Available targets:" -ForegroundColor Cyan
    Write-Host "  clean   - Remove build artifacts (build/ and bin/ directories)"
    Write-Host "  debug   - Build debug version with symbols"
    Write-Host "  release - Build release version (optimized)"
    Write-Host "  test    - Build debug version and run tests"
    Write-Host "  example - Build debug version and compile example program"
    Write-Host "  run     - Run compiler on a source file (builds if needed)"
    Write-Host "  help    - Show this help message"
    Write-Host ""
    Write-Host "Usage: .\make [target] [arguments...]" -ForegroundColor White
    Write-Host "Examples:" -ForegroundColor White
    Write-Host "  .\make clean" -ForegroundColor Gray
    Write-Host "  .\make release" -ForegroundColor Gray
    Write-Host "  .\make run MathScript-Compiler-main\test\examples\simple.mathseq -ast" -ForegroundColor Gray
}

# Main switch
switch ($Target.ToLower()) {
    "clean" {
        Clean-Build
    }
    "debug" {
        Build-Debug
    }
    "release" {
        Build-Release
    }
    "all" {
        Build-Release
    }
    "test" {
        Run-Test
    }
    "example" {
        Run-Example
    }
    "run" {
        Run-Compiler -args $ExtraArgs
    }
    "help" {
        Show-Help
    }
    default {
        Write-Host "Unknown target: $Target" -ForegroundColor Red
        Write-Host ""
        Show-Help
        exit 1
    }
}

