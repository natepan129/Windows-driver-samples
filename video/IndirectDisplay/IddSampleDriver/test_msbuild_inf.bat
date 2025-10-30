@echo off
echo ========================================
echo Test MSBuild INF with Working install_driver.exe
echo ========================================
echo.

set INSTALL_TOOL=%CD%\install_driver.exe
set INF_PATH=%CD%\x64\Release\IddSampleDriver.inf

if not exist "%INSTALL_TOOL%" (
    echo ERROR: install_driver.exe not found
    pause
    exit /b 1
)

if not exist "%INF_PATH%" (
    echo ERROR: MSBuild INF not found at %INF_PATH%
    pause
    exit /b 1
)

echo Using:
echo   Tool: %INSTALL_TOOL%
echo   INF: %INF_PATH%
echo.
echo Running installation with admin rights...
echo (UAC prompt will appear)
echo.
pause

cd /d "%~dp0"
powershell -Command "Start-Process -FilePath '%INSTALL_TOOL%' -ArgumentList '\""%INF_PATH%"\"' -Verb RunAs -Wait"

echo.
echo Installation complete. Checking result...
timeout /t 2 >nul
echo.

powershell -Command "Get-PnpDevice | Where-Object { $_.InstanceId -like '*IddSampleDriver*' -or $_.FriendlyName -like '*IddSampleDriver*' } | Format-Table FriendlyName, Status, Class, ClassGuid -AutoSize"

echo.
pause

