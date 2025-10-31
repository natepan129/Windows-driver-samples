@echo off
echo ========================================
echo Testing vddctl Full Cycle
echo ========================================
echo.

cd /d "%~dp0"

echo [1/4] Installing driver...
echo ----------------------------------------
build\bin\Release\vddctl.exe install --inf "%CD%\IddSampleDriver_Fixed.inf"
echo.

echo [2/4] Checking installed device...
echo ----------------------------------------
powershell -Command "Get-PnpDevice | Where-Object { $_.InstanceId -like '*IddSampleDriver*' } | Format-Table FriendlyName, Status, InstanceId, Class, ClassGuid -AutoSize"
echo.

echo [3/4] Uninstalling driver...
echo ----------------------------------------
build\bin\Release\vddctl.exe uninstall
echo.

echo [4/4] Verifying device removed...
echo ----------------------------------------
powershell -Command "$devices = Get-PnpDevice | Where-Object { $_.InstanceId -like '*IddSampleDriver*' }; if ($devices) { Write-Host 'ERROR: Devices still exist'; $devices | Format-Table } else { Write-Host 'SUCCESS: All devices removed' }"
echo.

echo ========================================
echo Test Complete!
echo ========================================
pause

