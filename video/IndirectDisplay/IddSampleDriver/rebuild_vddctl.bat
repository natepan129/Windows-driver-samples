@echo off
echo ========================================
echo Rebuilding vddctl.exe
echo ========================================

call "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat" >nul 2>&1

echo Compiling vddctl...
msbuild /p:Configuration=Release /p:Platform=x64 /t:vddctl /nologo /v:minimal

if %ERRORLEVEL% EQU 0 (
    echo.
    echo [SUCCESS] vddctl.exe rebuilt successfully
    echo Location: build\bin\Release\vddctl.exe
) else (
    echo.
    echo [FAILED] Build failed with error code %ERRORLEVEL%
)

echo.
pause

