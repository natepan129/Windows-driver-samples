@echo off
:: Check for admin
net session >nul 2>&1
if %errorLevel% == 0 (
    echo Running as Administrator
) else (
    echo ERROR: Not running as Administrator!
    echo Right-click this file and select "Run as administrator"
    pause
    exit /b 1
)

echo.
echo ========================================
echo Testing vddctl uninstall with Admin
echo ========================================
echo.

echo Devices BEFORE:
powershell -Command "Get-PnpDevice | Where-Object { $_.InstanceId -like '*IddSampleDriver*' } | Format-Table -AutoSize"

echo.
echo Running uninstall...
.\build\bin\Release\vddctl.exe uninstall

echo.
echo Devices AFTER:
powershell -Command "Get-PnpDevice | Where-Object { $_.InstanceId -like '*IddSampleDriver*' } | Format-Table -AutoSize"

echo.
echo ==========================================
pause

