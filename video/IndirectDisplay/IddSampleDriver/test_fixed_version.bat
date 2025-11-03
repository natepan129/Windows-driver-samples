@echo off
setlocal

echo ========================================
echo Testing Fixed vddsdk.cpp Version
echo ========================================
echo.
echo Changes implemented:
echo 1. Fixed duplicate device check (enumerate ALL devices)
echo 2. UninstallDriver: Remove DIGCF_PRESENT (catch offline nodes)
echo 3. Exact HWID matching (_wcsicmp)
echo 4. InstallDriver: Rollback on failure
echo 5. IsDriverInstalled: Removed pnputil, use registry + enumeration
echo 6. Concurrent operation protection (install/uninstall mutex)
echo 7. INF path converted to absolute path
echo.
echo ========================================

set "VDDCTL=.\build\bin\Release\vddctl.exe"
set "INF=.\IddSampleDriver.inf"

:: Check admin
whoami /groups | find "S-1-5-32-544" > nul
if %errorlevel% neq 0 (
    echo ERROR: Administrator privileges required!
    pause
    exit /b 1
)

:: Check files exist
if not exist "%VDDCTL%" (
    echo ERROR: vddctl.exe not found. Please run build script first.
    pause
    exit /b 1
)

if not exist "%INF%" (
    echo ERROR: INF file not found.
    pause
    exit /b 1
)

echo.
echo [Test 1: Clean existing devices]
echo ========================================
echo Current IddSampleDriver devices:
powershell -Command "Get-PnpDevice -Class Display | Where-Object { $_.InstanceId -like '*Idd*' } | Format-Table FriendlyName, Status, InstanceId -AutoSize"
echo.
echo Attempting full cleanup...
"%VDDCTL%" uninstall
echo.
pause

echo.
echo [Test 2: Verify all devices removed]
echo ========================================
echo Devices after uninstall (should be empty):
powershell -Command "Get-PnpDevice -Class Display | Where-Object { $_.InstanceId -like '*Idd*' } | Format-Table FriendlyName, Status, InstanceId -AutoSize"
echo.
pause

echo.
echo [Test 3: First installation]
echo ========================================
echo Installing driver (should succeed)...
"%VDDCTL%" install --inf "%INF%"
echo.
echo Devices after first install:
powershell -Command "Get-PnpDevice -Class Display | Where-Object { $_.InstanceId -like '*Idd*' } | Format-Table FriendlyName, Status, InstanceId -AutoSize"
echo.
pause

echo.
echo [Test 4: Duplicate prevention test]
echo ========================================
echo Attempting second install (should be prevented)...
echo Expected: "Device already installed" error
"%VDDCTL%" install --inf "%INF%"
echo.
echo Devices after duplicate attempt (should still be 1):
powershell -Command "Get-PnpDevice -Class Display | Where-Object { $_.InstanceId -like '*Idd*' } | Format-Table FriendlyName, Status, InstanceId -AutoSize"
echo.
pause

echo.
echo [Test 5: Safe uninstall with Disable first]
echo ========================================
echo Attempting uninstall (should disable then remove)...
"%VDDCTL%" uninstall
echo.
echo Devices after uninstall (should be empty):
powershell -Command "Get-PnpDevice -Class Display | Where-Object { $_.InstanceId -like '*Idd*' } | Format-Table FriendlyName, Status, InstanceId -AutoSize"
echo.
pause

echo.
echo [Test 6: Full cycle reinstall]
echo ========================================
echo Reinstalling after complete removal...
"%VDDCTL%" install --inf "%INF%"
echo.
echo Final device status:
powershell -Command "Get-PnpDevice -Class Display | Where-Object { $_.InstanceId -like '*Idd*' } | Format-Table FriendlyName, Status, InstanceId, Class, ClassGuid -AutoSize"
echo.

echo.
echo ========================================
echo All tests completed!
echo ========================================
echo.
echo Summary of fixes:
echo - Duplicate check now enumerates ALL devices
echo - Uninstall catches offline/phantom devices
echo - Exact HWID matching prevents false positives
echo - Rollback on installation failure
echo - Reliable driver detection
echo - Thread-safe operations
echo ========================================
pause
endlocal

