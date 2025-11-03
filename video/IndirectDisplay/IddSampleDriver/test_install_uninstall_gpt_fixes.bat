@echo off
:: Check for admin
net session >nul 2>&1
if %errorLevel% neq 0 (
    echo ERROR: Not running as Administrator!
    echo Right-click this file and select "Run as administrator"
    pause
    exit /b 1
)

echo ==========================================
echo Testing Install/Uninstall with GPT Fixes
echo ==========================================
echo.

echo [STEP 1] Check current devices
powershell -Command "Get-PnpDevice | Where-Object { $_.InstanceId -like '*IddSampleDriver*' } | Format-Table -AutoSize"
echo.

echo [STEP 2] Test Install
.\build\bin\Release\vddctl.exe install --inf x64\Release\IddSampleDriver\IddSampleDriver.inf
echo.

echo [STEP 3] Verify installation
powershell -Command "Get-PnpDevice | Where-Object { $_.InstanceId -like '*IddSampleDriver*' } | Format-Table -AutoSize"
echo.

pause
echo.

echo [STEP 4] Test Uninstall
.\build\bin\Release\vddctl.exe uninstall
echo.

echo [STEP 5] Verify uninstallation
powershell -Command "Get-PnpDevice | Where-Object { $_.InstanceId -like '*IddSampleDriver*' } | Format-Table -AutoSize"
echo.

echo ==========================================
echo Test Complete
echo ==========================================
pause

