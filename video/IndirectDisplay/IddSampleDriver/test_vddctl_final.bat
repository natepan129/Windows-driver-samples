@echo off
echo ========================================
echo VDD SDK Final Verification Test
echo Using vddctl with IddSampleDriver_Fixed.inf
echo ========================================
echo.

set VDDCTL=build\bin\Release\vddctl.exe
set INF_PATH=%CD%\IddSampleDriver_Fixed.inf

if not exist "%VDDCTL%" (
    echo ERROR: vddctl.exe not found
    pause
    exit /b 1
)

if not exist "%INF_PATH%" (
    echo ERROR: IddSampleDriver_Fixed.inf not found
    pause
    exit /b 1
)

echo Test Configuration:
echo   Tool: %VDDCTL%
echo   INF: %INF_PATH%
echo   DLL: x64\Release\IddSampleDriver.dll (MSBuild Release)
echo.

echo ========================================
echo Test 1: Version Check
echo ========================================
"%VDDCTL%" version
if %ERRORLEVEL% NEQ 0 (
    echo [FAIL] Version check failed
    pause
    exit /b 1
)
echo [PASS] Version check
echo.
pause

echo ========================================
echo Test 2: SDK Initialize
echo ========================================
"%VDDCTL%" init --verbose
if %ERRORLEVEL% NEQ 0 (
    echo [FAIL] SDK initialization failed
    pause
    exit /b 1
)
echo [PASS] SDK initialized
echo.
pause

echo ========================================
echo Test 3: Check Initial Status
echo ========================================
"%VDDCTL%" status
echo.
pause

echo ========================================
echo Test 4: Install Driver (needs admin)
echo ========================================
echo Using IddSampleDriver_Fixed.inf (correct encoding)
echo with x64\Release\IddSampleDriver.dll (MSBuild Release)
echo.
echo UAC prompt will appear - click YES
echo.
pause

powershell -Command "Start-Process '%VDDCTL%' -ArgumentList 'install --inf \""%INF_PATH%"\"' -Verb RunAs -Wait"

echo.
echo Waiting for installation...
timeout /t 3 >nul

echo ========================================
echo Test 5: Verify Installation
echo ========================================
echo.
echo [A] vddctl status:
"%VDDCTL%" status
echo.

echo [B] Device Manager check:
powershell -Command "Get-PnpDevice | Where-Object { $_.InstanceId -like '*IddSampleDriver*' -or $_.FriendlyName -like '*IddSampleDriver*' } | Format-Table FriendlyName, Status, Class, ClassGuid -AutoSize"
echo.
pause

echo ========================================
echo Final Verification Summary
echo ========================================
echo.
echo Please check the results above:
echo.
echo  [1] vddctl version - should show version info
echo  [2] vddctl init - should succeed
echo  [3] vddctl install - should succeed
echo  [4] vddctl status - should show "Driver Installed: Yes"
echo  [5] Device - should have "Display" class
echo.
echo If all checks pass, VDD SDK basic functions work correctly!
echo.
echo Next steps:
echo   - Test activate: vddctl activate --width 1920 --height 1080
echo   - Test deactivate: vddctl deactivate  
echo   - Test uninstall: vddctl uninstall
echo.
pause

