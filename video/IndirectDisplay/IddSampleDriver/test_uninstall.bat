@echo off
cd /d "%~dp0"

echo ========================================
echo Testing vddctl uninstall
echo ========================================
echo.

echo [1] Current Status:
echo ----------------------------------------
vddctl.exe status
echo.
pause

echo.
echo [2] Uninstalling driver...
echo ----------------------------------------
vddctl.exe uninstall
echo.
pause

echo.
echo [3] Status After Uninstall:
echo ----------------------------------------
vddctl.exe status
echo.

echo ========================================
echo Test Complete
echo ========================================
pause

@echo off
cd /d "%~dp0"

echo ========================================
echo Testing vddctl uninstall
echo ========================================
echo.

echo [1] Current Status:
echo ----------------------------------------
vddctl.exe status
echo.
pause

echo.
echo [2] Uninstalling driver...
echo ----------------------------------------
vddctl.exe uninstall
echo.
pause

echo.
echo [3] Status After Uninstall:
echo ----------------------------------------
vddctl.exe status
echo.

echo ========================================
echo Test Complete
echo ========================================
pause

