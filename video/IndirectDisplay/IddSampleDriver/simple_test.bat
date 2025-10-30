@echo off
echo ========================================
echo Simple Install and Uninstall Test
echo ========================================
echo.

echo [1] Installing driver...
install_driver.exe IddSampleDriver_Fixed.inf
timeout /t 2 >nul

echo.
echo [2] Checking devices...
powershell -NoProfile -Command "Get-PnpDevice | Where-Object { $_.InstanceId -like '*IddSampleDriver*' } | Format-Table FriendlyName, Status, Class -AutoSize"

echo.
echo Press any key to uninstall...
pause

echo.
echo [3] Uninstalling driver...
uninstall_driver.exe
timeout /t 2 >nul

echo.
echo [4] Checking devices after uninstall...
powershell -NoProfile -Command "Get-PnpDevice | Where-Object { $_.InstanceId -like '*IddSampleDriver*' } | Format-Table FriendlyName, Status, Class -AutoSize"

echo.
echo Done!
pause

