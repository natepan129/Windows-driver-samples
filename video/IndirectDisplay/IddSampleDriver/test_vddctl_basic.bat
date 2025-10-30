@echo off
echo ========================================
echo VDD Control Tool - Basic Function Test
echo ========================================
echo.

set VDDCTL=build\bin\Release\vddctl.exe

REM Check if vddctl exists
if not exist "%VDDCTL%" (
    echo ERROR: vddctl.exe not found at %VDDCTL%
    echo Please build the project first.
    pause
    exit /b 1
)

echo Using: %VDDCTL%
echo.

echo ========================================
echo Test 1: Check VDD SDK Version
echo ========================================
"%VDDCTL%" version
echo.
pause

echo ========================================
echo Test 2: Check Driver Status
echo ========================================
"%VDDCTL%" status
echo.
pause

echo ========================================
echo Test 3: Initialize SDK
echo ========================================
"%VDDCTL%" init --verbose
echo.
pause

echo ========================================
echo Test 4: Check Driver Installation
echo ========================================
echo (This checks if driver is installed)
"%VDDCTL%" status
echo.
pause

echo ========================================
echo Test 5: List Displays and Adapters
echo ========================================
"%VDDCTL%" list
echo.
pause

echo ========================================
echo Test Summary
echo ========================================
echo.
echo Completed basic function tests:
echo   [1] Version check
echo   [2] Status check
echo   [3] SDK initialization
echo   [4] Driver status
echo   [5] Display/adapter enumeration
echo.
echo Next steps:
echo   - If driver not installed: vddctl install --inf IddSampleDriver_Fixed.inf
echo   - To activate display: vddctl activate --width 1920 --height 1080
echo   - To deactivate: vddctl deactivate
echo   - To uninstall: vddctl uninstall
echo.
pause

