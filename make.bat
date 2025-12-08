@echo off
REM Windows batch wrapper for make.ps1
REM Usage: make [target]

powershell -ExecutionPolicy Bypass -File "%~dp0make.ps1" %*

