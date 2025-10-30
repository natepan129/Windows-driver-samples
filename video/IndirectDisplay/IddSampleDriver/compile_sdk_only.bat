@echo off
echo ========================================
echo Compile VDD SDK Tools Only
echo (vddsdk.dll + vddctl.exe)
echo ========================================
echo.

call "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat" >nul 2>&1

echo Compiling vddsdk.dll...
cl.exe /EHsc /std:c++17 /O2 /MD /DUNICODE /D_UNICODE /LD ^
    /Febuild\bin\Release\vddsdk.dll ^
    vddsdk.cpp ^
    setupapi.lib newdev.lib ole32.lib advapi32.lib user32.lib kernel32.lib cfgmgr32.lib

if %ERRORLEVEL% NEQ 0 (
    echo [FAILED] vddsdk.dll compilation failed
    pause
    exit /b 1
)

echo [OK] vddsdk.dll compiled
echo.

echo Compiling vddctl.exe...
cl.exe /EHsc /std:c++17 /O2 /MD /DUNICODE /D_UNICODE ^
    /Febuild\bin\Release\vddctl.exe ^
    vddctl.cpp ^
    build\bin\Release\vddsdk.lib ^
    setupapi.lib newdev.lib ole32.lib advapi32.lib user32.lib kernel32.lib cfgmgr32.lib

if %ERRORLEVEL% NEQ 0 (
    echo [FAILED] vddctl.exe compilation failed
    pause
    exit /b 1
)

echo [OK] vddctl.exe compiled
echo.

echo ========================================
echo SUCCESS
echo ========================================
echo.
echo Output:
echo   build\bin\Release\vddsdk.dll
echo   build\bin\Release\vddsdk.lib
echo   build\bin\Release\vddctl.exe
echo.

pause

