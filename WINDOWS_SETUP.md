# Windows Setup Instructions for MathSeq Compiler

## Current Status
✅ WSL (Windows Subsystem for Linux) and Ubuntu have been installed
⚠️ System reboot required before WSL can be used

## Option 1: Using WSL (Recommended - After Reboot)

### Step 1: Reboot your system
The system needs to be rebooted for WSL to become active.

### Step 2: After reboot, initialize Ubuntu
```powershell
wsl
```
This will prompt you to set up a username and password for Ubuntu.

### Step 3: Install build tools in WSL
Once inside WSL/Ubuntu, run:
```bash
sudo apt update
sudo apt install -y build-essential
```

### Step 4: Build and run the project
From PowerShell (in the project directory):
```powershell
# Build
wsl make release

# Run examples
wsl ./bin/mathseqc test/examples/fibonacci.mathseq -ast -output fibonacci.asm
wsl ./bin/mathseqc test/examples/simple.mathseq -ast
wsl ./bin/mathseqc test/examples/arithmetic.mathseq -ast
```

Or use the run script:
```powershell
powershell -ExecutionPolicy Bypass -File run.ps1
```

## Option 2: Using Native Windows Compiler (No Reboot Needed)

### Install MinGW-w64
1. Download from: https://www.mingw-w64.org/downloads/
2. Or install via installer: https://sourceforge.net/projects/mingw-w64/
3. Add `bin` directory to your PATH environment variable

### Then build and run:
```powershell
# Build
powershell -ExecutionPolicy Bypass -File build.ps1 -Mode release

# Run examples
.\bin\mathseqc.exe test\examples\fibonacci.mathseq -ast -output fibonacci.asm
.\bin\mathseqc.exe test\examples\simple.mathseq -ast
.\bin\mathseqc.exe test\examples\arithmetic.mathseq -ast
```

Or use the run script:
```powershell
powershell -ExecutionPolicy Bypass -File run.ps1
```

## Quick Start (After WSL is Ready)

Once WSL is set up, you can use:

```powershell
# Quick build and test
wsl make release
wsl make test

# Compile all examples
powershell -ExecutionPolicy Bypass -File run.ps1
```

## Available Example Files
- `test/examples/simple.mathseq` - Simple program
- `test/examples/fibonacci.mathseq` - Basic Fibonacci sequence generator
- `test/examples/fibonacci_squared.mathseq` - Fibonacci + map square demo
- `test/examples/fibonacci_pattern.mathseq` - Fibonacci pattern detection
- `test/examples/arithmetic.mathseq` - Arithmetic operations
- `test/examples/errors.mathseq` - Error handling examples

## Troubleshooting

### WSL not working after reboot
- Ensure virtualization is enabled in BIOS
- Run: `wsl --update`
- Check: `wsl --status`

### Build errors
- Ensure you have C++17 support
- Check that all source files are present in `src/` directory
- Verify include files exist in `include/` directory

