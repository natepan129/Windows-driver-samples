@echo off
echo ========================================
echo VDD Complete Test Script
echo ========================================

echo.
echo Step 1: Initialize SDK...
build\bin\Release\vddctl.exe init
if %errorlevel% neq 0 (
    echo ERROR: SDK initialization failed
    pause
    exit /b 1
)

echo.
echo Step 2: Install Driver...
build\bin\Release\vddctl.exe install
if %errorlevel% neq 0 (
    echo ERROR: Driver installation failed
    pause
    exit /b 1
)

echo.
echo Step 3: Check Status...
build\bin\Release\vddctl.exe status

echo.
echo Step 4: Activate Virtual Display...
build\bin\Release\vddctl.exe activate --name "Virtual Display 1" --width 1920 --height 1080 --count 1
if %errorlevel% neq 0 (
    echo ERROR: Virtual display activation failed
    pause
    exit /b 1
)

echo.
echo Step 5: List Active Displays...
build\bin\Release\vddctl.exe list

echo.
echo Step 6: Set Display Mode...
build\bin\Release\vddctl.exe setmode --output 0 --width 1920 --height 1080 --refresh 60

echo.
echo Step 7: Set Display Location...
build\bin\Release\vddctl.exe setlocation --output 0 --x 0 --y 0

echo.
echo Step 8: Set Primary Display...
build\bin\Release\vddctl.exe setprimary --output 0

echo.
echo ========================================
echo Test completed successfully!
echo ========================================
pause
