@echo off
echo ========================================
echo Build vddctl (Static Linking)
echo ========================================
echo.

REM Setup Visual Studio environment
call "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat" >nul 2>&1

if not exist "build\bin\Release" mkdir build\bin\Release

echo Compiling vddctl.exe with vddsdk...
echo ----------------------------------------

cl.exe /EHsc /std:c++17 /O2 /MD /DUNICODE /D_UNICODE ^
    /I. ^
    /Fobuild\bin\Release\ ^
    /Febuild\bin\Release\vddctl.exe ^
    vddctl.cpp vddsdk.cpp ^
    setupapi.lib newdev.lib ole32.lib advapi32.lib user32.lib kernel32.lib cfgmgr32.lib

if %ERRORLEVEL% NEQ 0 (
    echo [FAILED] Compilation failed
    pause
    exit /b 1
)

echo.
echo ========================================
echo Build Complete!
echo ========================================
echo.
echo Output: build\bin\Release\vddctl.exe
echo.

echo Testing vddctl...
echo ----------------------------------------
build\bin\Release\vddctl.exe version

echo.
echo Success! vddctl is ready to use.
echo.
echo Quick test commands:
echo   build\bin\Release\vddctl.exe init
echo   build\bin\Release\vddctl.exe status
echo.
pause

