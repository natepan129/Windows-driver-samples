@echo off
echo ========================================
echo Test Cycle: Install and Uninstall
echo ========================================
echo.

echo Step 1: Install driver
echo ========================================
install_driver.exe IddSampleDriver_Fixed.inf
timeout /t 2 >nul

echo.
echo Step 2: Check devices
echo ========================================
cmd /c check_devices.bat

echo.
echo Step 3: Press any key to uninstall...
pause

echo.
echo Step 4: Uninstall driver
echo ========================================
uninstall_driver.exe
timeout /t 2 >nul

echo.
echo Step 5: Check devices after uninstall
echo ========================================
cmd /c check_devices.bat

echo.
echo ========================================
echo Test Complete!
echo ========================================
pause

