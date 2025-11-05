@echo off
cd /d "%~dp0"
echo ========================================
echo Testing vddctl install with DEBUG output
echo ========================================
echo.
echo Current directory: %CD%
echo.
.\vddctl.exe install .\x64\Release\IddSampleDriver\IddSampleDriver.inf
echo.
echo ========================================
echo.
pause









