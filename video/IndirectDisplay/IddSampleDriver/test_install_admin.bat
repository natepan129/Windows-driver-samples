@echo off
echo ========================================
echo Test Install with Admin Rights
echo ========================================
echo.
echo Current directory: %CD%
echo.

REM Use full path to ensure exe can be found
"%~dp0install_driver.exe" "%~dp0IddSampleDriver_Fixed.inf"

echo.
echo ========================================
echo Installation attempt complete
echo ========================================
echo.

timeout /t 2 >nul

echo Checking devices...
powershell -NoProfile -Command "Get-PnpDevice | Where-Object { $_.InstanceId -like '*IddSampleDriver*' } | Format-Table FriendlyName, Status, Class -AutoSize"

echo.
pause

