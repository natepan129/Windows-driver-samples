@echo off
setlocal

set "VDDCTL=.\build\bin\Release\vddctl.exe"

echo ========================================
echo Remove All IddSampleDriver Devices
echo ========================================
echo.

:: Check admin
whoami /groups | find "S-1-5-32-544" > nul
if %errorlevel% neq 0 (
    echo ERROR: Administrator privileges required!
    pause
    exit /b 1
)
echo Admin: OK
echo.

echo Current devices:
powershell -Command "Get-PnpDevice -Class Display | Where-Object { $_.InstanceId -like '*Idd*' } | Format-Table FriendlyName, Status, InstanceId -AutoSize"
echo.
pause

echo ========================================
echo Step 1: Initialize SDK
echo ========================================
"%VDDCTL%" init
echo.
pause

echo ========================================
echo Step 2: Uninstall All Devices
echo ========================================
"%VDDCTL%" uninstall
echo.
pause

echo ========================================
echo Verification: Check Devices After Uninstall
echo ========================================
powershell -Command "$devices = Get-PnpDevice -Class Display | Where-Object { $_.InstanceId -like '*Idd*' }; if ($devices) { $devices | Format-Table FriendlyName, Status, InstanceId -AutoSize } else { Write-Host 'SUCCESS: No IddSampleDriver devices found!' -ForegroundColor Green }"
echo.

echo ========================================
echo Complete!
echo ========================================
pause
endlocal

