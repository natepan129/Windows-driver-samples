@echo off
chcp 65001 >nul
echo ========================================
echo VDD Direct Commands (No PowerShell)
echo ========================================

set VDD_DIR=%~dp0
set DRIVER_DIR=%VDD_DIR%x64\Debug\IddSampleDriver

echo [INFO] VDD Directory: %VDD_DIR%
echo [INFO] Driver Directory: %DRIVER_DIR%

echo.
echo ========================================
echo Step 1: Check Administrator Privileges
echo ========================================

net session >nul 2>&1
if %errorLevel% == 0 (
    echo [SUCCESS] Administrator privileges confirmed
) else (
    echo [ERROR] Administrator privileges required
    echo [INFO] Please run as administrator
    pause
    exit /b 1
)

echo.
echo ========================================
echo Step 2: Check Driver Files
echo ========================================

if exist "%DRIVER_DIR%\IddSampleDriver.dll" (
    echo [SUCCESS] Found driver DLL: IddSampleDriver.dll
) else (
    echo [ERROR] Driver DLL not found
    echo [INFO] Please check file path
    pause
    exit /b 1
)

if exist "%DRIVER_DIR%\IddSampleDriver.inf" (
    echo [SUCCESS] Found driver INF: IddSampleDriver.inf
) else (
    echo [ERROR] Driver INF not found
    echo [INFO] Please check file path
    pause
    exit /b 1
)

echo.
echo ========================================
echo Step 3: Install VDD Driver
echo ========================================

echo [INFO] Installing VDD driver...
echo [INFO] Using pnputil to install driver...

pnputil /add-driver "%DRIVER_DIR%\IddSampleDriver.inf" /install

if %errorLevel% == 0 (
    echo [SUCCESS] Driver installed successfully!
) else (
    echo [WARNING] Driver installation may have failed
    echo [INFO] Error code: %errorLevel%
)

echo.
echo ========================================
echo Step 4: Verify Installation
echo ========================================

echo [INFO] Checking if driver is installed...
pnputil /enum-drivers | findstr "IddSampleDriver"
if %errorLevel% == 0 (
    echo [SUCCESS] Driver successfully installed
) else (
    echo [WARNING] Driver may not be installed correctly
)

echo.
echo ========================================
echo Step 5: Test VDD Functionality
echo ========================================

if exist "%VDD_DIR%\x64\Debug\IddSampleApp.exe" (
    echo [INFO] Found sample application, starting test...
    start "" "%VDD_DIR%\x64\Debug\IddSampleApp.exe"
    echo [SUCCESS] Sample application started
) else (
    echo [WARNING] Sample application not found
)

echo.
echo ========================================
echo Step 6: Next Steps
echo ========================================

echo.
echo Installation complete! Please perform the following:
echo.
echo 1. Check Windows Display Settings:
echo    - Open Windows Settings
echo    - System > Display
echo    - Look for new virtual displays
echo.
echo 2. If no virtual displays are visible:
echo    - Try restarting the system
echo    - Check Windows Device Manager
echo    - View Windows Event Viewer
echo.
echo 3. Test virtual display functionality:
echo    - Run .\x64\Debug\IddSampleApp.exe
echo    - Check if virtual display works properly
echo.

echo ========================================
echo Installation Complete
echo ========================================
echo.
echo Thank you for using VDD Direct Commands!
echo.
pause
