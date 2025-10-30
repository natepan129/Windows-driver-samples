@echo off
echo ========================================
echo Compiling vddctl.exe
echo ========================================

call "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat" >nul 2>&1

echo Compiling vddctl.cpp...
cl.exe /EHsc /std:c++17 /O2 /MT ^
    /DUNICODE /D_UNICODE ^
    /Fobuild\bin\Release\vddctl.obj ^
    /Febuild\bin\Release\vddctl.exe ^
    vddctl.cpp ^
    build\bin\Release\vddsdk.lib ^
    setupapi.lib ole32.lib advapi.lib user32.lib

if %ERRORLEVEL% EQU 0 (
    echo.
    echo [SUCCESS] vddctl.exe compiled successfully
    echo Location: build\bin\Release\vddctl.exe
) else (
    echo.
    echo [FAILED] Compilation failed
)

echo.
pause

