@echo off
echo ========================================
echo Test vddctl install with Debug Output
echo ========================================
echo.

set VDDCTL=%CD%\build\bin\Release\vddctl.exe
set INF=%CD%\IddSampleDriver_Fixed.inf
set LOG=%CD%\vddctl_install_log.txt

echo Configuration:
echo   vddctl: %VDDCTL%
echo   INF: %INF%
echo   Log: %LOG%
echo.

echo Running vddctl install...
echo (This will request admin rights)
echo.
pause

REM Run with output redirect
powershell -Command "Start-Process -FilePath '%VDDCTL%' -ArgumentList 'install --inf %INF%' -Verb RunAs -RedirectStandardOutput '%LOG%' -RedirectStandardError '%LOG%_error.txt' -Wait"

echo.
echo Installation complete. Checking output...
echo.

echo ========================================
echo Output:
echo ========================================
type "%LOG%" 2>nul
echo.

echo ========================================
echo Errors:
echo ========================================
type "%LOG%_error.txt" 2>nul
echo.

echo ========================================
echo Device Status:
echo ========================================
powershell -Command "Get-PnpDevice | Where-Object { $_.FriendlyName -like '*IddSampleDriver*' } | Format-Table FriendlyName, Status, Class -AutoSize"

echo.
pause

