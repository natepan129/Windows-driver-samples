@echo off
echo ========================================
echo Quick Install with vddctl
echo ========================================
echo.

echo Step 1: Cleaning old devices...
powershell -Command "Start-Process '%CD%\uninstall_driver.exe' -Verb RunAs -Wait"
timeout /t 2 >nul
echo [OK]
echo.

echo Step 2: Installing driver (first time)...
powershell -Command "Start-Process '%CD%\build\bin\Release\vddctl.exe' -ArgumentList 'install --inf %CD%\IddSampleDriver_Fixed.inf' -Verb RunAs -Wait"
timeout /t 2 >nul
echo [OK]
echo.

echo Step 3: Installing driver (second time for Display Class)...
powershell -Command "Start-Process '%CD%\build\bin\Release\vddctl.exe' -ArgumentList 'install --inf %CD%\IddSampleDriver_Fixed.inf' -Verb RunAs -Wait"
timeout /t 2 >nul
echo [OK]
echo.

echo ========================================
echo Verification
echo ========================================
echo.

echo vddctl status:
build\bin\Release\vddctl.exe status
echo.

echo Devices:
powershell -Command "Get-PnpDevice | Where-Object { $_.FriendlyName -like '*IddSampleDriver*' } | Format-Table FriendlyName, Status, Class, InstanceId -AutoSize"

echo.
echo ========================================
echo Installation Complete!
echo ========================================
echo.
echo You should see at least one device with "Display" class above.
echo.
pause

