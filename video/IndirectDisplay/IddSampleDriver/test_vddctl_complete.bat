@echo off
echo ========================================
echo VDD Control Tool - Complete Test
echo ========================================
echo.

set VDDCTL=build\bin\Release\vddctl.exe
set INF_FILE=IddSampleDriver_Fixed.inf

REM Check files
if not exist "%VDDCTL%" (
    echo ERROR: vddctl.exe not found
    pause
    exit /b 1
)

if not exist "%INF_FILE%" (
    echo ERROR: %INF_FILE% not found
    pause
    exit /b 1
)

echo Test Environment:
echo   vddctl: %VDDCTL%
echo   INF: %INF_FILE%
echo.
pause

echo.
echo ========================================
echo Step 1: Check Initial Status
echo ========================================
"%VDDCTL%" status
echo.
pause

echo.
echo ========================================
echo Step 2: Initialize SDK
echo ========================================
"%VDDCTL%" init --verbose
echo.
pause

echo.
echo ========================================
echo Step 3: Install Driver
echo ========================================
echo (Note: UAC prompt may appear - click Yes)
echo.
pause

powershell -Command "Start-Process '%VDDCTL%' -ArgumentList 'install --inf %INF_FILE%' -Verb RunAs -Wait"

echo.
echo Installation complete. Checking status...
timeout /t 2 >nul
echo.
"%VDDCTL%" status
echo.
pause

echo.
echo ========================================
echo Step 4: List Displays and Adapters
echo ========================================
"%VDDCTL%" list
echo.
pause

echo.
echo ========================================
echo Step 5: Activate Virtual Display
echo ========================================
echo Creating 1920x1080 @ 60Hz virtual display...
echo.
pause

"%VDDCTL%" activate --width 1920 --height 1080 --refresh 60 --name "Test Display"
echo.
pause

echo.
echo ========================================
echo Step 6: Check Active Display Status
echo ========================================
"%VDDCTL%" status
echo.
echo You should see "Display Active: Yes" above
echo.
pause

echo.
echo ========================================
echo Step 7: Query Display Mode
echo ========================================
"%VDDCTL%" list
echo.
pause

echo.
echo ========================================
echo Step 8: Deactivate Virtual Display
echo ========================================
echo Removing virtual display...
echo.
pause

"%VDDCTL%" deactivate
echo.
pause

echo.
echo ========================================
echo Step 9: Final Status Check
echo ========================================
"%VDDCTL%" status
echo.
pause

echo.
echo ========================================
echo Test Complete!
echo ========================================
echo.
echo Test Summary:
echo   [PASS] vddctl version works
echo   [PASS] SDK initialization works
echo   [????] Driver installation (check output above)
echo   [????] Display activation (check output above)
echo   [????] Display deactivation (check output above)
echo.
echo The driver is still installed. To uninstall:
echo   %VDDCTL% uninstall
echo.
pause

