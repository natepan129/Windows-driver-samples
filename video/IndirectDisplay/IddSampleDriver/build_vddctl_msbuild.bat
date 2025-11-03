@echo off
echo ========================================
echo Build vddctl using MSBuild
echo ========================================
echo.

REM Setup Visual Studio environment
call "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat" >nul 2>&1

echo Building vddctl.vcxproj...
echo ----------------------------------------

msbuild vddctl.vcxproj /p:Configuration=Release /p:Platform=x64 /nologo /v:minimal

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo [FAILED] Build failed
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
echo Success! vddctl is ready.
echo.
pause



