@echo off
setlocal

echo ========================================
echo Testing VDD with CORRECT Command Order
echo ========================================
echo.
echo Design Doc Required Order:
echo 1. vddctl init
echo 2. vddctl install
echo 3. vddctl status
echo 4. vddctl uninstall
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
echo [Step 1: Initialize SDK]
echo ========================================
"%VDDCTL%" init
echo.
pause

echo.
echo [Step 2: Check current devices]
echo ========================================
powershell -Command "Get-PnpDevice -Class Display | Where-Object { $_.InstanceId -like '*Idd*' } | Format-Table FriendlyName, Status, InstanceId -AutoSize"
echo.
pause

echo.
echo [Step 3: Uninstall (with SDK already initialized)]
echo ========================================
"%VDDCTL%" uninstall
echo.
pause

echo.
echo [Step 4: Verify devices removed]
echo ========================================
powershell -Command "Get-PnpDevice -Class Display | Where-Object { $_.InstanceId -like '*Idd*' } | Format-Table FriendlyName, Status, InstanceId -AutoSize"
echo.
pause

echo.
echo [Step 5: Install (with SDK already initialized)]
echo ========================================
"%VDDCTL%" install --inf "%INF%"
echo.
pause

echo.
echo [Step 6: Verify installation]
echo ========================================
"%VDDCTL%" status
echo.
powershell -Command "Get-PnpDevice -Class Display | Where-Object { $_.InstanceId -like '*Idd*' } | Format-Table FriendlyName, Status, InstanceId, Class -AutoSize"
echo.

echo ========================================
echo Test Complete
echo ========================================
pause
endlocal

