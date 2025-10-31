@echo off
echo ========================================
echo Build vddctl Complete Package
echo ========================================
echo.

REM Setup Visual Studio environment
call "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat" >nul 2>&1

if not exist "build\bin\Release" mkdir build\bin\Release

echo Step 1/2: Compiling vddsdk.dll...
echo ----------------------------------------

cl.exe /EHsc /std:c++17 /O2 /MD /DUNICODE /D_UNICODE /DVDD_SDK_EXPORTS ^
    /I. ^
    /LD ^
    /Fobuild\bin\Release\vddsdk.obj ^
    /Febuild\bin\Release\vddsdk.dll ^
    vddsdk.cpp ^
    setupapi.lib newdev.lib ole32.lib advapi32.lib user32.lib kernel32.lib cfgmgr32.lib ^
    /link /DEF:vddsdk.def /IMPLIB:build\bin\Release\vddsdk.lib

if %ERRORLEVEL% NEQ 0 (
    echo [FAILED] vddsdk.dll compilation failed
    pause
    exit /b 1
)

echo [OK] vddsdk.dll compiled successfully
echo.

echo Step 2/2: Compiling vddctl.exe...
echo ----------------------------------------

cl.exe /EHsc /std:c++17 /O2 /MD /DUNICODE /D_UNICODE ^
    /I. ^
    /Fobuild\bin\Release\vddctl.obj ^
    /Febuild\bin\Release\vddctl.exe ^
    vddctl.cpp ^
    build\bin\Release\vddsdk.lib ^
    setupapi.lib newdev.lib ole32.lib advapi32.lib user32.lib kernel32.lib cfgmgr32.lib

if %ERRORLEVEL% NEQ 0 (
    echo [FAILED] vddctl.exe compilation failed
    pause
    exit /b 1
)

echo [OK] vddctl.exe compiled successfully
echo.

echo ========================================
echo Build Complete!
echo ========================================
echo.
echo Output files:
dir /b build\bin\Release\vddsdk.dll 2>nul
dir /b build\bin\Release\vddsdk.lib 2>nul
dir /b build\bin\Release\vddctl.exe 2>nul
echo.

echo Testing vddctl...
echo ----------------------------------------
build\bin\Release\vddctl.exe version

echo.
echo Next steps:
echo   1. Test init:    build\bin\Release\vddctl.exe init
echo   2. Test install: build\bin\Release\vddctl.exe install --inf IddSampleDriver_Fixed.inf
echo   3. Test status:  build\bin\Release\vddctl.exe status
echo.
pause

