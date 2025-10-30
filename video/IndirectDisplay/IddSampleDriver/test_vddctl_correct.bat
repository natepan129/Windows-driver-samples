@echo off
echo ========================================
echo VDD SDK Complete Test - Using Correct INF
echo ========================================
echo.

set VDDCTL=build\bin\Release\vddctl.exe
set INF_PATH=%CD%\x64\Release\IddSampleDriver.inf

REM Check files exist
if not exist "%VDDCTL%" (
    echo ERROR: vddctl.exe not found at %VDDCTL%
    echo Please build the SDK first: msbuild /p:Configuration=Release /p:Platform=x64
    pause
    exit /b 1
)

if not exist "%INF_PATH%" (
    echo ERROR: IddSampleDriver.inf not found at %INF_PATH%
    echo Please build the driver first: msbuild /p:Configuration=Release /p:Platform=x64
    pause
    exit /b 1
)

echo Files OK:
echo   vddctl: %VDDCTL%
echo   INF: %INF_PATH%
echo.

echo ========================================
echo Step 1: Check Version
echo ========================================
"%VDDCTL%" version
echo.
pause

echo ========================================
echo Step 2: Initialize SDK
echo ========================================
"%VDDCTL%" init --verbose
echo.
pause

echo ========================================
echo Step 3: Check Initial Status
echo ========================================
"%VDDCTL%" status
echo.
pause

echo ========================================
echo Step 4: Install Driver (needs admin)
echo ========================================
echo.
echo Using MSBuild INF: %INF_PATH%
echo UAC prompt will appear - click YES
echo.
pause

powershell -Command "Start-Process '%VDDCTL%' -ArgumentList 'install --inf \""%INF_PATH%"\"' -Verb RunAs -Wait"

echo.
echo Waiting for installation to complete...
timeout /t 3 >nul

echo ========================================
echo Step 5: Verify Installation
echo ========================================
echo.
echo Checking vddctl status:
"%VDDCTL%" status
echo.

echo Checking Device Manager:
powershell -Command "Get-PnpDevice | Where-Object { $_.InstanceId -like '*IddSampleDriver*' -or $_.FriendlyName -like '*IddSampleDriver*' } | Format-Table FriendlyName, Status, Class, ClassGuid -AutoSize"
echo.
pause

echo ========================================
echo Test Complete!
echo ========================================
echo.
echo Check the results above:
echo   - vddctl status should show "Driver Installed: Yes"
echo   - Device should have "Display" class
echo.
echo If successful, the VDD SDK basic functions work correctly!
echo.
echo To clean up:
echo   "%VDDCTL%" uninstall
echo.
pause

