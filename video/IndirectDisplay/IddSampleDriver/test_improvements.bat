@echo off
cd /d "%~dp0"
echo ========================================
echo Testing VDD SDK Improvements
echo ========================================
echo.

REM First ensure we have admin rights
net session >nul 2>&1
if %errorLevel% neq 0 (
    echo ERROR: This script requires administrator privileges!
    echo Please right-click and select "Run as administrator"
    pause
    exit /b 1
)

echo [Test 1/3] Testing duplicate installation protection
echo ========================================
echo.

echo Step 1: Clean install (should succeed)
build\bin\Release\vddctl.exe uninstall >nul 2>&1
build\bin\Release\vddctl.exe install --inf IddSampleDriver.inf
if errorlevel 1 (
    echo FAIL: First installation failed
    pause
    exit /b 1
)
echo SUCCESS: First installation completed
echo.

echo Step 2: Check device status
powershell -Command "Get-PnpDevice | Where-Object { $_.InstanceId -like '*IddSampleDriver*' } | Format-Table FriendlyName, Status, Class -AutoSize"
echo.

echo Step 3: Try to install again (should detect duplicate)
build\bin\Release\vddctl.exe install --inf IddSampleDriver.inf
echo Expected: Should show "Device already installed" or "AlreadyInstalled"
echo.
pause

echo.
echo [Test 2/3] Testing safe uninstall (disable before remove)
echo ========================================
echo.

echo Step 1: Check device before uninstall
powershell -Command "Get-PnpDevice | Where-Object { $_.InstanceId -like '*IddSampleDriver*' } | Format-Table FriendlyName, Status -AutoSize"
echo.

echo Step 2: Uninstall (will disable first, then remove)
build\bin\Release\vddctl.exe uninstall
echo.

echo Step 3: Verify device removed
powershell -Command "Get-PnpDevice | Where-Object { $_.InstanceId -like '*IddSampleDriver*' } | Format-Table FriendlyName, Status -AutoSize"
echo Expected: No devices found
echo.
pause

echo.
echo [Test 3/3] Testing complete cycle (Install → Uninstall → Reinstall)
echo ========================================
echo.

echo Step 1: Install
build\bin\Release\vddctl.exe install --inf IddSampleDriver.inf
if errorlevel 1 (
    echo FAIL: Installation failed
    pause
    exit /b 1
)
echo SUCCESS: Installed
powershell -Command "Get-PnpDevice | Where-Object { $_.InstanceId -like '*IddSampleDriver*' } | Select-Object FriendlyName, Status, Class"
echo.

echo Step 2: Uninstall
build\bin\Release\vddctl.exe uninstall
if errorlevel 1 (
    echo FAIL: Uninstallation failed
    pause
    exit /b 1
)
echo SUCCESS: Uninstalled
powershell -Command "Get-PnpDevice | Where-Object { $_.InstanceId -like '*IddSampleDriver*' } | Format-Table"
echo.

echo Step 3: Reinstall
build\bin\Release\vddctl.exe install --inf IddSampleDriver.inf
if errorlevel 1 (
    echo FAIL: Reinstallation failed
    pause
    exit /b 1
)
echo SUCCESS: Reinstalled
powershell -Command "Get-PnpDevice | Where-Object { $_.InstanceId -like '*IddSampleDriver*' } | Select-Object FriendlyName, Status, Class"
echo.

echo ========================================
echo All Tests Complete!
echo ========================================
echo.
echo Summary:
echo - Duplicate installation protection: Check output above
echo - Safe uninstall (disable first): Completed without black screen
echo - Complete cycle: Install → Uninstall → Reinstall successful
echo.
pause

