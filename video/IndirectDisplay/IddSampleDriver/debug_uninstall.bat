@echo off
setlocal

set "VDDCTL=.\build\bin\Release\vddctl.exe"

echo ========================================
echo Debug UninstallDriver Issue
echo ========================================
echo.

:: Check admin
whoami /groups | find "S-1-5-32-544" > nul
if %errorlevel% neq 0 (
    echo ERROR: Not running as administrator!
    pause
    exit /b 1
)
echo Admin: OK
echo.

echo ========================================
echo Test 1: Check vddctl version
echo ========================================
"%VDDCTL%" version
echo.
pause

echo ========================================
echo Test 2: Initialize SDK
echo ========================================
"%VDDCTL%" init
echo.
pause

echo ========================================
echo Test 3: Check status
echo ========================================
"%VDDCTL%" status
echo.
pause

echo ========================================
echo Test 4: Call UninstallDriver
echo ========================================
"%VDDCTL%" uninstall --verbose
echo.
pause

echo ========================================
echo Test 5: Check devices again
echo ========================================
powershell -Command "Get-PnpDevice -Class Display | Where-Object { $_.InstanceId -like '*Idd*' } | Format-Table FriendlyName, Status, InstanceId -AutoSize"
echo.

echo ========================================
echo Test 6: Try direct PowerShell removal
echo ========================================
echo Attempting to remove devices using pnputil...
pnputil /enum-drivers | findstr /i "iddsampledriver"
echo.

pause
endlocal

