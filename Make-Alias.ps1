# PowerShell function to enable 'make' command
# Run this in your PowerShell session: . .\Make-Alias.ps1

function make {
    param([Parameter(ValueFromRemainingArguments=$true)][string[]]$targets)
    
    if ($targets.Count -eq 0) {
        & "$PSScriptRoot\make.bat" help
    } else {
        & "$PSScriptRoot\make.bat" $targets
    }
}

Write-Host "Make function loaded! You can now use 'make clean', 'make debug', etc." -ForegroundColor Green
Write-Host "To use this in future sessions, run: . .\Make-Alias.ps1" -ForegroundColor Yellow

