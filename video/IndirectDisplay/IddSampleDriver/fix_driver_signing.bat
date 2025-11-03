@echo off
echo ========================================
echo Driver Signing Diagnostic and Fix
echo ========================================
echo.

echo [1] Checking current test signing status...
bcdedit /enum {current} | findstr /i "testsigning"
if %ERRORLEVEL% EQU 0 (
    echo Status: Test signing is ENABLED
) else (
    echo Status: Test signing is DISABLED
)
echo.

echo [2] Enabling test signing mode...
bcdedit /set testsigning on
if %ERRORLEVEL% EQU 0 (
    echo Result: SUCCESS
) else (
    echo Result: FAILED - Make sure you run as Administrator
    pause
    exit /b 1
)
echo.

echo [3] Disabling driver signature enforcement...
bcdedit /set nointegritychecks on
echo.

echo ========================================
echo Configuration Updated Successfully
echo ========================================
echo.
echo CRITICAL: You MUST REBOOT now!
echo.
echo After reboot, you will see:
echo   - "Test Mode" watermark in bottom-right corner
echo   - This is normal and expected
echo.
echo Then run: test_install.bat
echo.

pause

echo.
echo Do you want to reboot now? (Y/N)
set /p REBOOT=
if /i "%REBOOT%"=="Y" (
    echo Rebooting in 10 seconds...
    shutdown /r /t 10 /c "Reboot required for test signing mode"
) else (
    echo.
    echo Remember to reboot manually before testing!
)

@echo off
echo ========================================
echo Driver Signing Diagnostic and Fix
echo ========================================
echo.

echo [1] Checking current test signing status...
bcdedit /enum {current} | findstr /i "testsigning"
if %ERRORLEVEL% EQU 0 (
    echo Status: Test signing is ENABLED
) else (
    echo Status: Test signing is DISABLED
)
echo.

echo [2] Enabling test signing mode...
bcdedit /set testsigning on
if %ERRORLEVEL% EQU 0 (
    echo Result: SUCCESS
) else (
    echo Result: FAILED - Make sure you run as Administrator
    pause
    exit /b 1
)
echo.

echo [3] Disabling driver signature enforcement...
bcdedit /set nointegritychecks on
echo.

echo ========================================
echo Configuration Updated Successfully
echo ========================================
echo.
echo CRITICAL: You MUST REBOOT now!
echo.
echo After reboot, you will see:
echo   - "Test Mode" watermark in bottom-right corner
echo   - This is normal and expected
echo.
echo Then run: test_install.bat
echo.

pause

echo.
echo Do you want to reboot now? (Y/N)
set /p REBOOT=
if /i "%REBOOT%"=="Y" (
    echo Rebooting in 10 seconds...
    shutdown /r /t 10 /c "Reboot required for test signing mode"
) else (
    echo.
    echo Remember to reboot manually before testing!
)

