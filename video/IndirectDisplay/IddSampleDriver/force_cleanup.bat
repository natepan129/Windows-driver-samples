@echo off
cd /d "%~dp0"
echo ========================================
echo Force Cleanup All IddSampleDriver Devices
echo ========================================
echo.

REM Check admin
net session >nul 2>&1
if %errorLevel% neq 0 (
    echo ERROR: Administrator privileges required!
    echo Please right-click and select "Run as administrator"
    pause
    exit /b 1
)
echo Administrator check: OK
echo.

echo Current devices BEFORE cleanup:
echo ----------------------------------------
powershell -Command "Get-PnpDevice | Where-Object { $_.InstanceId -like '*IddSampleDriver*' } | Format-Table FriendlyName, Status, InstanceId -AutoSize"
echo.

echo Attempting to uninstall (will show detailed output)...
echo ----------------------------------------
build\bin\Release\vddctl.exe uninstall
echo Exit code: %ERRORLEVEL%
echo.

echo Current devices AFTER first attempt:
echo ----------------------------------------
powershell -Command "Get-PnpDevice | Where-Object { $_.InstanceId -like '*IddSampleDriver*' } | Format-Table FriendlyName, Status, InstanceId -AutoSize"
echo.

echo If devices still exist, trying again...
echo ----------------------------------------
build\bin\Release\vddctl.exe uninstall
echo Exit code: %ERRORLEVEL%
echo.

echo Current devices AFTER second attempt:
echo ----------------------------------------
powershell -Command "Get-PnpDevice | Where-Object { $_.InstanceId -like '*IddSampleDriver*' } | Format-Table FriendlyName, Status, InstanceId -AutoSize"
echo.

echo If devices STILL exist, trying third time...
echo ----------------------------------------
build\bin\Release\vddctl.exe uninstall
echo Exit code: %ERRORLEVEL%
echo.

echo FINAL device count:
echo ----------------------------------------
powershell -Command "$devices = Get-PnpDevice | Where-Object { $_.InstanceId -like '*IddSampleDriver*' }; $devices | Format-Table FriendlyName, Status, InstanceId -AutoSize; Write-Host 'Total devices remaining:' $devices.Count"
echo.

echo ========================================
echo Cleanup Complete!
echo ========================================
pause

