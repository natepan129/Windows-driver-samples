@echo off
cd /d "%~dp0"

echo ========================================
echo VDD Control Tool - Command Line Tests
echo ========================================
echo.

echo [Test 1] Check Status
echo ----------------------------------------
vddctl.exe status
echo.
pause

echo.
echo [Test 2] List Adapters
echo ----------------------------------------
vddctl.exe list
echo.
pause

echo.
echo [Test 3] Enumerate Modes
echo ----------------------------------------
vddctl.exe enum-modes
echo.
pause

echo.
echo [Test 4] Driver Info (if available)
echo ----------------------------------------
vddctl.exe info
echo.
pause

echo.
echo ========================================
echo All basic tests completed!
echo ========================================
echo.
echo Next steps to test (require additional setup):
echo   - vddctl activate   (activate virtual display)
echo   - vddctl deactivate (deactivate virtual display)
echo   - vddctl setmode --width 1920 --height 1080
echo   - vddctl uninstall  (uninstall driver)
echo.
pause

@echo off
cd /d "%~dp0"

echo ========================================
echo VDD Control Tool - Command Line Tests
echo ========================================
echo.

echo [Test 1] Check Status
echo ----------------------------------------
vddctl.exe status
echo.
pause

echo.
echo [Test 2] List Adapters
echo ----------------------------------------
vddctl.exe list
echo.
pause

echo.
echo [Test 3] Enumerate Modes
echo ----------------------------------------
vddctl.exe enum-modes
echo.
pause

echo.
echo [Test 4] Driver Info (if available)
echo ----------------------------------------
vddctl.exe info
echo.
pause

echo.
echo ========================================
echo All basic tests completed!
echo ========================================
echo.
echo Next steps to test (require additional setup):
echo   - vddctl activate   (activate virtual display)
echo   - vddctl deactivate (deactivate virtual display)
echo   - vddctl setmode --width 1920 --height 1080
echo   - vddctl uninstall  (uninstall driver)
echo.
pause

