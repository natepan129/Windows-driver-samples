@echo off
echo ========================================
echo Quick Install Test (with proper paths)
echo ========================================
echo.

REM Get script directory
set SCRIPT_DIR=%~dp0
echo Script location: %SCRIPT_DIR%
echo.

REM Check if files exist
if not exist "%SCRIPT_DIR%install_driver.exe" (
    echo ERROR: install_driver.exe not found!
    echo Looking in: %SCRIPT_DIR%
    pause
    exit /b 1
)

if not exist "%SCRIPT_DIR%IddSampleDriver_Fixed.inf" (
    echo ERROR: IddSampleDriver_Fixed.inf not found!
    echo Looking in: %SCRIPT_DIR%
    pause
    exit /b 1
)

echo Files found, running installation...
echo.
echo (Note: This will show UAC prompt - click Yes)
echo.
pause

REM Run with full paths and request admin rights
powershell -Command "Start-Process -FilePath '%SCRIPT_DIR%install_driver.exe' -ArgumentList '%SCRIPT_DIR%IddSampleDriver_Fixed.inf' -Verb RunAs -Wait"

echo.
echo ========================================
echo Installation complete, checking result...
echo ========================================
echo.
timeout /t 2 >nul

powershell -NoProfile -Command "Get-PnpDevice | Where-Object { $_.InstanceId -like '*IddSampleDriver*' } | Format-Table FriendlyName, Status, Class -AutoSize"

echo.
echo ========================================
echo Test complete
echo ========================================
pause

