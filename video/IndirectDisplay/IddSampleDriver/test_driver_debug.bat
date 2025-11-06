@echo off
echo ========================================
echo Testing IddSampleDriver with Debug Output
echo ========================================
echo.
echo Step 1: Uninstall old driver
echo ----------------------------------------
vddctl.exe uninstall
timeout /t 2 /nobreak >nul

echo.
echo Step 2: Install new driver
echo ----------------------------------------
vddctl.exe install "x64\Release\IddSampleDriver\IddSampleDriver.inf"
timeout /t 3 /nobreak >nul

echo.
echo Step 3: Check status
echo ----------------------------------------
vddctl.exe status

echo.
echo Step 4: List displays
echo ----------------------------------------
vddctl.exe list

echo.
echo ========================================
echo Test completed!
echo ========================================
echo.
echo Next steps:
echo 1. Download DebugView: https://learn.microsoft.com/en-us/sysinternals/downloads/debugview
echo 2. Run DebugView as Administrator
echo 3. Enable "Capture Kernel" in menu
echo 4. Look for [IddSample] messages
echo.
echo Or check Event Viewer ^(eventvwr.msc^) -^> Windows Logs -^> System
echo Search for "IddCx" or "IndirectDisplay"
echo.
pause








