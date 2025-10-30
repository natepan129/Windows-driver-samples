@echo off
echo ========================================
echo Install IddSampleDriver using vddctl
echo ========================================
echo.

REM Set paths
set VDDCTL=%CD%\build\bin\Release\vddctl.exe
set INF_PATH=%CD%\IddSampleDriver_Fixed.inf

REM Check if vddctl exists
if not exist "%VDDCTL%" (
    echo ERROR: vddctl.exe not found at:
    echo   %VDDCTL%
    echo.
    echo Please build it first:
    echo   cmd /c compile_sdk_only.bat
    pause
    exit /b 1
)

REM Check if INF exists
if not exist "%INF_PATH%" (
    echo ERROR: IddSampleDriver_Fixed.inf not found at:
    echo   %INF_PATH%
    pause
    exit /b 1
)

echo Configuration:
echo   vddctl: %VDDCTL%
echo   INF:    %INF_PATH%
echo   DLL:    x64\Release\IddSampleDriver.dll
echo.

echo ========================================
echo Step 1: Initialize SDK
echo ========================================
"%VDDCTL%" init
echo.
timeout /t 1 >nul

echo ========================================
echo Step 2: Check Status Before Install
echo ========================================
"%VDDCTL%" status
echo.
pause

echo ========================================
echo Step 3: Install Driver
echo ========================================
echo.
echo Running: vddctl install --inf "%INF_PATH%"
echo.
echo NOTE: UAC prompt will appear - Click YES to allow
echo.
pause

REM Run vddctl with admin rights
powershell -Command "Start-Process -FilePath '%VDDCTL%' -ArgumentList 'install --inf \"%INF_PATH%\"' -Verb RunAs -Wait"

echo.
echo Installation command completed
echo Waiting 3 seconds for system to update...
timeout /t 3 >nul

echo ========================================
echo Step 4: Verify Installation
echo ========================================
echo.
echo Checking vddctl status:
"%VDDCTL%" status
echo.

echo Checking Device Manager:
powershell -Command "Get-PnpDevice | Where-Object { $_.InstanceId -like '*IddSampleDriver*' -or $_.FriendlyName -like '*IddSampleDriver*' } | Format-Table FriendlyName, Status, Class, ClassGuid -AutoSize"
echo.

echo ========================================
echo Installation Complete!
echo ========================================
echo.
echo Expected results:
echo   - vddctl status shows "Driver Installed: Yes"
echo   - Device appears in Device Manager
echo   - Device has "Display" class
echo   - Device status is "OK"
echo.
echo If successful, the driver is ready to use!
echo.
echo Next steps:
echo   - Activate display: vddctl activate --width 1920 --height 1080
echo   - List displays: vddctl list
echo   - Deactivate: vddctl deactivate
echo   - Uninstall: vddctl uninstall
echo.
pause

