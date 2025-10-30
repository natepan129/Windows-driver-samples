@echo off
echo ========================================
echo VDD SDK Basic Functionality Test
echo ========================================
echo.

REM Setup Visual Studio environment
set "VS_PATH=C:\Program Files (x86)\Microsoft Visual Studio\2019"
if not exist "%VS_PATH%" set "VS_PATH=C:\Program Files\Microsoft Visual Studio\2022"

if exist "%VS_PATH%\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" (
    call "%VS_PATH%\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x64
) else if exist "%VS_PATH%\Community\VC\Auxiliary\Build\vcvarsall.bat" (
    call "%VS_PATH%\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
) else (
    echo Error: Visual Studio not found
    goto :manual_compile
)

:compile
echo.
echo Compiling test program...
cl.exe /nologo /O2 /W3 test_basic_sdk_functions.cpp setupapi.lib cfgmgr32.lib user32.lib /link /out:test_sdk_basic.exe

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo Compilation failed
    pause
    exit /b 1
)

echo Compilation successful!
echo.
goto :run

:manual_compile
cl.exe /nologo /O2 /W3 test_basic_sdk_functions.cpp setupapi.lib cfgmgr32.lib user32.lib /link /out:test_sdk_basic.exe 2>nul
if %ERRORLEVEL% EQU 0 goto :run

echo.
echo Cannot compile. Please run from Visual Studio Developer Command Prompt
pause
exit /b 1

:run
echo ========================================
echo Running Basic Functionality Tests
echo ========================================
echo.
echo Note: This test checks if:
echo   - Driver is installed
echo   - Device has correct properties
echo   - System is stable
echo   - Driver is safe to use
echo.
pause

REM Run the test
test_sdk_basic.exe

echo.
echo ========================================
echo Test Complete
echo ========================================
pause

