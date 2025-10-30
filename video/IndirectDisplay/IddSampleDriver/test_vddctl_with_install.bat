@echo off
echo ========================================
echo VDD SDK Complete Functionality Test
echo Using vddctl (Design Document Tool)
echo ========================================
echo.

set VDDCTL=build\bin\Release\vddctl.exe
set INF_PATH=%CD%\IddSampleDriver_Fixed.inf

echo Step 1: Check Version
echo ========================================
"%VDDCTL%" version
echo.

echo Step 2: Initialize SDK
echo ========================================
"%VDDCTL%" init
echo.

echo Step 3: Install Driver (requires admin)
echo ========================================
echo.
echo About to install driver with INF: %INF_PATH%
echo This requires administrator rights.
echo Please click YES on the UAC prompt.
echo.
pause

powershell -Command "Start-Process '%VDDCTL%' -ArgumentList 'install --inf %INF_PATH%' -Verb RunAs -Wait"

echo.
echo Installation complete. Waiting 3 seconds...
timeout /t 3 >nul

echo.
echo Step 4: Check Driver Status
echo ========================================
"%VDDCTL%" status
echo.
timeout /t 2 >nul

echo.
echo Step 5: Verify Device in Device Manager
echo ========================================
powershell -NoProfile -Command "Get-PnpDevice | Where-Object { $_.InstanceId -like '*IddSampleDriver*' } | Format-Table FriendlyName, Status, Class -AutoSize"
echo.

echo ========================================
echo Basic Tests Complete!
echo ========================================
echo.
echo Results:
echo   [1] Version: Check output above
echo   [2] Init: Check output above
echo   [3] Install: Check output above
echo   [4] Status: Check output above
echo   [5] Device: Check output above
echo.
echo If "Driver Installed: Yes" and device has "Display" class,
echo then basic functions are working correctly!
echo.
echo Next: Test activate/deactivate?
echo   Run: test_vddctl_activate.bat
echo.
pause

