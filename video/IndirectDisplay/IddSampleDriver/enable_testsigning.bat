@echo off
echo ========================================
echo Enable Test Signing Mode
echo ========================================
echo.
echo This will allow installation of test-signed drivers
echo.

bcdedit /set testsigning on

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ========================================
    echo SUCCESS: Test signing enabled
    echo ========================================
    echo.
    echo IMPORTANT: You must REBOOT for this to take effect
    echo.
    echo After reboot, run test_install.bat again
    echo.
) else (
    echo.
    echo ========================================
    echo FAILED: Could not enable test signing
    echo ========================================
    echo.
    echo Make sure you are running as Administrator
    echo.
)

pause

@echo off
echo ========================================
echo Enable Test Signing Mode
echo ========================================
echo.
echo This will allow installation of test-signed drivers
echo.

bcdedit /set testsigning on

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ========================================
    echo SUCCESS: Test signing enabled
    echo ========================================
    echo.
    echo IMPORTANT: You must REBOOT for this to take effect
    echo.
    echo After reboot, run test_install.bat again
    echo.
) else (
    echo.
    echo ========================================
    echo FAILED: Could not enable test signing
    echo ========================================
    echo.
    echo Make sure you are running as Administrator
    echo.
)

pause

