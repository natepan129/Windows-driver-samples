@echo off
echo ========================================
echo Rebuild VDD SDK (vddsdk.dll + vddctl.exe)
echo ========================================
echo.

REM Setup Visual Studio environment
call "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat" >nul 2>&1

echo Building Release version...
echo.

REM Build the SDK
msbuild /p:Configuration=Release /p:Platform=x64 /nologo /v:minimal

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ========================================
    echo Build SUCCESS
    echo ========================================
    echo.
    echo Output files:
    dir /b build\bin\Release\vddsdk.dll 2>nul
    dir /b build\bin\Release\vddctl.exe 2>nul
    echo.
) else (
    echo.
    echo ========================================
    echo Build FAILED
    echo ========================================
    echo Error code: %ERRORLEVEL%
)

pause

