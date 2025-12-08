# Installing a C++ Compiler on Windows

This guide will help you install a C++ compiler (g++) on Windows so you can build the MathScript compiler.

## Option 1: MSYS2 (Recommended - No Admin Required)

MSYS2 is a popular distribution that includes MinGW and doesn't require admin rights for installation.

1. **Download MSYS2:**
   - Visit: https://www.msys2.org/
   - Download the installer (e.g., `msys2-x86_64-latest.exe`)

2. **Install MSYS2:**
   - Run the installer
   - Install to a location you have write access to (e.g., `C:\msys64` or your user directory)
   - Complete the installation

3. **Open MSYS2 terminal and install MinGW:**
   ```
   pacman -Syu
   pacman -S mingw-w64-x86_64-gcc
   pacman -S mingw-w64-x86_64-make
   ```

4. **Add to PATH:**
   - Find the MSYS2 installation directory (default: `C:\msys64`)
   - Add `C:\msys64\mingw64\bin` to your user PATH:
     - Press `Win + R`, type `sysdm.cpl`, press Enter
     - Click "Environment Variables"
     - Under "User variables", find "Path" and click "Edit"
     - Click "New" and add: `C:\msys64\mingw64\bin`
     - Click OK on all dialogs
   - **Restart PowerShell** after adding to PATH

5. **Verify installation:**
   ```
   g++ --version
   ```

## Option 2: Chocolatey (Requires Admin Rights)

If you have administrator rights:

1. **Open PowerShell as Administrator:**
   - Right-click PowerShell
   - Select "Run as Administrator"

2. **Install MinGW:**
   ```
   choco install mingw -y
   ```

3. **Restart PowerShell** and verify:
   ```
   g++ --version
   ```

## Option 3: Portable MinGW (No Admin Required)

You can download a portable version of MinGW:

1. **Download MinGW-w64:**
   - Visit: https://winlibs.com/
   - Download the latest release (e.g., "winlibs-x86_64-posix-seh-gcc-*-mingw-w64-*-msvcrt-*.zip")
   - Extract to a folder (e.g., `C:\mingw64` or your user directory)

2. **Add to PATH:**
   - Add the `bin` folder to your user PATH:
     - Example: `C:\mingw64\bin`
   - **Restart PowerShell**

3. **Verify installation:**
   ```
   g++ --version
   ```

## Option 4: Visual Studio Build Tools (No Admin Required for User Install)

Microsoft offers Visual Studio Build Tools:

1. **Download Visual Studio Build Tools:**
   - Visit: https://visualstudio.microsoft.com/downloads/
   - Scroll down to "All Downloads" → "Tools for Visual Studio" → "Build Tools for Visual Studio"

2. **Install:**
   - Run the installer
   - Select "Desktop development with C++" workload
   - Install to your user directory

3. **Use Developer Command Prompt:**
   - Find "Developer Command Prompt for VS" in Start Menu
   - This sets up the environment automatically

## Option 5: Winget (Windows 10/11 - May Require Admin)

If you have Windows 10/11 with winget:

```powershell
winget install mingw-w64
```

Then restart PowerShell and verify.

## After Installation

Once you have g++ installed, restart PowerShell and try building:

```powershell
.\make release
```

The script will automatically detect your compiler and use it.

## Troubleshooting

### "g++ not found" after installation
- Make sure you **restarted PowerShell** after adding to PATH
- Verify PATH: `$env:PATH -split ';' | Select-String mingw`
- Check if g++ exists: `Get-Command g++ -ErrorAction SilentlyContinue`

### Still having issues?
- Edit `make.ps1` and manually set the compiler path:
  ```powershell
  $CXX = "C:\path\to\g++.exe"
  ```


