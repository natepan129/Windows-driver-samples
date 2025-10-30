@echo off
echo ========================================
echo Build and Run SetupAPI Installation Test
echo ========================================
echo.

REM Check if we have cl.exe in path
where cl.exe >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: cl.exe not found in PATH
    echo Please run this from a Visual Studio Developer Command Prompt
    echo Or run vcvarsall.bat first
    pause
    exit /b 1
)

echo Compiling test_setupapi_install.cpp...
cl.exe /EHsc /W4 /std:c++17 test_setupapi_install.cpp setupapi.lib newdev.lib

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ERROR: Compilation failed
    pause
    exit /b 1
)

echo.
echo Compilation successful!
echo.
echo ========================================
echo Running as Administrator...
echo ========================================
echo.

REM Run as administrator
powershell -Command "Start-Process -FilePath '%CD%\test_setupapi_install.exe' -Verb RunAs -Wait"

echo.
echo ========================================
echo Test Complete
echo ========================================
pause


