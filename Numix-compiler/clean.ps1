# PowerShell equivalent of 'make clean'
# Removes build and bin directories

$buildDir = "build"
$binDir = "bin"

Write-Host "Cleaning build artifacts..."

if (Test-Path $buildDir) {
    Remove-Item -Path $buildDir -Recurse -Force
    Write-Host "Removed $buildDir directory"
} else {
    Write-Host "$buildDir directory does not exist"
}

if (Test-Path $binDir) {
    Remove-Item -Path $binDir -Recurse -Force
    Write-Host "Removed $binDir directory"
} else {
    Write-Host "$binDir directory does not exist"
}

Write-Host "Clean complete!"

