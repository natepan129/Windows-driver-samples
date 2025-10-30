@echo off
echo ========================================
echo Quick Uninstall Test
echo ========================================
echo.

REM Get script directory
set SCRIPT_DIR=%~dp0
echo Script location: %SCRIPT_DIR%
echo.

REM Check if uninstall tool exists
if not exist "%SCRIPT_DIR%uninstall_driver.exe" (
    echo ERROR: uninstall_driver.exe not found!
    pause
    exit /b 1
)

echo Running uninstall...
echo (Note: This will show UAC prompt - click Yes)
echo.
pause

REM Run with admin rights
powershell -Command "Start-Process -FilePath '%SCRIPT_DIR%uninstall_driver.exe' -Verb RunAs -Wait"

echo.
echo ========================================
echo Uninstall complete, checking result...
echo ========================================
echo.
timeout /t 2 >nul

powershell -NoProfile -Command "Get-PnpDevice | Where-Object { $_.InstanceId -like '*IddSampleDriver*' } | Format-Table FriendlyName, Status, Class -AutoSize"

echo.
echo ========================================
echo Test complete
echo ========================================
pause

