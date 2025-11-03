@echo off
setlocal

echo ========================================
echo Diagnose UninstallDriver Issue
echo ========================================

set "VDDCTL=.\build\bin\Release\vddctl.exe"

:: Check admin
whoami /groups | find "S-1-5-32-544" > nul
if %errorlevel% neq 0 (
    echo ERROR: Not running as administrator!
    pause
    exit /b 1
)
echo Admin: OK
echo.

echo Devices BEFORE uninstall:
powershell -Command "Get-PnpDevice -Class Display | Where-Object { $_.InstanceId -like '*Idd*' } | Format-Table FriendlyName, Status, InstanceId -AutoSize"
echo.

echo Running: vddctl uninstall
"%VDDCTL%" uninstall
echo.

echo Devices AFTER uninstall:
powershell -Command "Get-PnpDevice -Class Display | Where-Object { $_.InstanceId -like '*Idd*' } | Format-Table FriendlyName, Status, InstanceId -AutoSize"
echo.

echo Checking registry:
reg query "HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Enum\ROOT\IddSampleDriver" 2>nul
if %errorlevel% equ 0 (
    echo Registry entries still exist!
) else (
    echo Registry entries removed
)
echo.

echo ========================================
pause
endlocal

