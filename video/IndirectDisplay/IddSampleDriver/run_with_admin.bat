@echo off
echo ========================================
echo Running Install with Administrator Rights
echo ========================================
echo.

REM Check for admin rights
net session >nul 2>&1
if %errorLevel% == 0 (
    echo Already running as administrator
    goto :run_install
) else (
    echo Requesting administrator rights...
    echo Right-click this script and select "Run as administrator"
    echo Or: The UAC prompt should appear - please click "Yes"
    pause
    exit /b 1
)

:run_install
echo.
echo Current directory: %CD%
echo Script directory: %~dp0
echo.

REM Change to script directory
cd /d "%~dp0"
echo Changed to: %CD%
echo.

echo Running installation...
"%~dp0install_driver.exe" "%~dp0IddSampleDriver_Fixed.inf"

echo.
timeout /t 2 >nul

echo.
echo Checking result...
powershell -NoProfile -Command "Get-PnpDevice | Where-Object { $_.InstanceId -like '*IddSampleDriver*' } | Format-Table FriendlyName, Status, Class -AutoSize"

echo.
pause

