@echo off
cd /d "%~dp0"
echo ========================================
echo Cleanup and Test Script
echo ========================================
echo.

REM Check admin
net session >nul 2>&1
if %errorLevel% neq 0 (
    echo ERROR: Administrator privileges required!
    pause
    exit /b 1
)

echo Step 1: Cleaning all IddSampleDriver devices...
echo ----------------------------------------
build\bin\Release\vddctl.exe uninstall
echo.

echo Step 2: Verify devices removed...
echo ----------------------------------------
powershell -Command "Get-PnpDevice | Where-Object { $_.InstanceId -like '*IddSampleDriver*' } | Format-Table FriendlyName, Status, InstanceId -AutoSize"
echo Expected: No devices found
echo.
pause

echo Step 3: First installation (should succeed)...
echo ----------------------------------------
build\bin\Release\vddctl.exe install --inf IddSampleDriver.inf
echo.

echo Step 4: Check device status...
echo ----------------------------------------
powershell -Command "Get-PnpDevice | Where-Object { $_.InstanceId -like '*IddSampleDriver*' } | Format-Table FriendlyName, Status, Class, InstanceId -AutoSize"
echo Expected: ONE device with Status=OK, Class=Display
echo.
pause

echo Step 5: Try to install AGAIN (should detect duplicate)...
echo ----------------------------------------
build\bin\Release\vddctl.exe install --inf IddSampleDriver.inf
echo Expected output: "Device already installed" or "AlreadyInstalled"
echo.
pause

echo Step 6: Verify still only ONE device...
echo ----------------------------------------
powershell -Command "Get-PnpDevice | Where-Object { $_.InstanceId -like '*IddSampleDriver*' } | Format-Table FriendlyName, Status, Class, InstanceId -AutoSize"
echo Expected: Still ONE device (no duplicates created)
echo.
pause

echo ========================================
echo Test Complete!
echo ========================================
pause

