@echo off
setlocal

echo ========================================
echo Testing VDD with Design Doc Order
echo ========================================
echo.
echo Correct Order (Design Doc):
echo   1. vddctl init
echo   2. vddctl install
echo   3. vddctl uninstall
echo.
echo ========================================

set "VDDCTL=.\build\bin\Release\vddctl.exe"
set "INF=.\IddSampleDriver.inf"

:: Check admin
whoami /groups | find "S-1-5-32-544" > nul
if %errorlevel% neq 0 (
    echo ERROR: Administrator privileges required!
    pause
    exit /b 1
)

echo.
echo ========================================
echo Step 1: Initialize SDK (only once)
echo ========================================
"%VDDCTL%" init
echo.
pause

echo.
echo ========================================
echo Current devices BEFORE uninstall:
echo ========================================
powershell -Command "Get-PnpDevice -Class Display | Where-Object { $_.InstanceId -like '*Idd*' } | Format-Table FriendlyName, Status, InstanceId -AutoSize"
echo.
pause

echo.
echo ========================================
echo Step 2: Uninstall all devices
echo ========================================
"%VDDCTL%" uninstall
echo.
pause

echo.
echo ========================================
echo Devices AFTER uninstall (should be empty):
echo ========================================
powershell -Command "Get-PnpDevice -Class Display | Where-Object { $_.InstanceId -like '*Idd*' } | Format-Table FriendlyName, Status, InstanceId -AutoSize"
echo.
pause

echo.
echo ========================================
echo Step 3: Install driver (fresh install)
echo ========================================
"%VDDCTL%" install --inf "%INF%"
echo.
pause

echo.
echo ========================================
echo Devices AFTER install (should be 1):
echo ========================================
powershell -Command "Get-PnpDevice -Class Display | Where-Object { $_.InstanceId -like '*Idd*' } | Format-Table FriendlyName, Status, InstanceId, Class -AutoSize"
echo.
pause

echo.
echo ========================================
echo Step 4: Try duplicate install (should fail)
echo ========================================
"%VDDCTL%" install --inf "%INF%"
echo.
pause

echo.
echo ========================================
echo Devices count (should still be 1):
echo ========================================
powershell -Command "Get-PnpDevice -Class Display | Where-Object { $_.InstanceId -like '*Idd*' } | Format-Table FriendlyName, Status, InstanceId -AutoSize"
echo.

echo.
echo ========================================
echo Test Complete!
echo ========================================
echo.
echo Results summary:
echo - SDK initialized: Check Step 1 output
echo - 3 devices removed: Check Step 2 output
echo - 1 device installed: Check Step 3 output
echo - Duplicate prevented: Check Step 4 output
echo.
pause
endlocal

