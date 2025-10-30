@echo off
REM Complete Windows Test Mode Enable and IddSampleDriver Installation

echo ========================================
echo Windows Test Mode Enable and IddSampleDriver Installation
echo ========================================

REM Check Administrator Privileges
net session >nul 2>&1
if %errorlevel% neq 0 (
    echo ERROR: Administrator privileges required
    echo Please right-click this file and select "Run as administrator"
    pause
    exit /b 1
)

echo SUCCESS: Administrator privileges confirmed

echo.
echo ========================================
echo Step 1: Check Current Test Mode Status
echo ========================================
echo Checking test signing status...
bcdedit /enum | findstr -i "testsigning"

echo.
echo ========================================
echo Step 2: Enable Test Mode
echo ========================================
echo Enabling test signing mode...
bcdedit /set testsigning on

echo Enabling developer mode...
bcdedit /set bootdebug on

echo.
echo ========================================
echo Step 3: Check Test Mode Status
echo ========================================
echo Re-checking test signing status...
bcdedit /enum | findstr -i "testsigning"

echo.
echo ========================================
echo Step 4: Clean Existing Installation
echo ========================================
echo Removing existing IddSampleDriver devices...
pnputil /remove-device "SWD\IddSampleDriver\IddSampleDriver" 2>nul
pnputil /remove-device "ROOT\IddSampleDriver" 2>nul

echo.
echo ========================================
echo Step 5: Install Driver
echo ========================================
echo Installing IddSampleDriver to ROOT location...
pnputil /add-driver "IddSampleDriver.inf" /install

echo.
echo ========================================
echo Step 6: Check Installation Results
echo ========================================
echo Checking ROOT devices:
pnputil /enum-devices | findstr -i "root.*idd"

echo.
echo Checking all IddSampleDriver devices:
pnputil /enum-devices | findstr -i "idd"

echo.
echo ========================================
echo Step 7: Enable Device
echo ========================================
echo Attempting to enable ROOT\IddSampleDriver device...
pnputil /enable-device "ROOT\IddSampleDriver"

echo.
echo ========================================
echo Step 8: Test SDK
echo ========================================
echo Testing VDD SDK...
build\bin\Release\vddctl.exe init
build\bin\Release\vddctl.exe status

echo.
echo ========================================
echo IMPORTANT REMINDER
echo ========================================
echo WARNING: System needs to restart to fully enable test mode
echo WARNING: After restart, you should see "Test Mode" watermark
echo WARNING: After restart, run this script again to check installation status
echo.
echo ========================================
echo COMPLETED
echo ========================================
pause

