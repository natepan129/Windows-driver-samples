@echo off
setlocal

set "VDDCTL=.\build\bin\Release\vddctl.exe"

echo ========================================
echo Test UninstallDriver (Fixed Version)
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

echo Current devices BEFORE uninstall:
powershell -Command "Get-PnpDevice -Class Display | Where-Object { $_.InstanceId -like '*Idd*' } | Format-Table FriendlyName, Status, InstanceId -AutoSize"
echo.
pause

echo ========================================
echo Calling: vddctl uninstall
echo (No need to init - standalone operation)
echo ========================================
"%VDDCTL%" uninstall
echo.
pause

echo ========================================
echo Checking devices AFTER uninstall:
echo ========================================
powershell -Command "$devices = Get-PnpDevice -Class Display | Where-Object { $_.InstanceId -like '*Idd*' }; if ($devices) { Write-Host 'ERROR: Devices still exist!' -ForegroundColor Red; $devices | Format-Table FriendlyName, Status, InstanceId -AutoSize } else { Write-Host 'SUCCESS: All devices removed!' -ForegroundColor Green }"
echo.

echo ========================================
echo Complete!
echo ========================================
pause
endlocal

