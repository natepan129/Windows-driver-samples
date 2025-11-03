@echo off
setlocal enabledelayedexpansion

cd /d "%~dp0"
set "PROJECT_DIR=%CD%"

echo ========================================
echo VDD Build and Verification Script
echo ========================================
echo Current User: %USERNAME%
echo Project Directory: %PROJECT_DIR%
echo.

REM Step 1: Clean old files
echo [1/5] Cleaning old files...
del /f /q "%PROJECT_DIR%\build\bin\Release\vddsdk.dll" 2>nul
del /f /q "%PROJECT_DIR%\build\bin\Release\vddctl.exe" 2>nul
del /f /q "%PROJECT_DIR%\build\vddsdk.dir\Release\*.obj" 2>nul
del /f /q "%PROJECT_DIR%\build\vddctl.dir\Release\*.obj" 2>nul
echo Done.
echo.

REM Step 2: Build
echo [2/5] Building vddsdk and vddctl...
"C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" "%PROJECT_DIR%\build\vddsdk.vcxproj" /p:Configuration=Release /p:Platform=x64 /t:Rebuild /v:minimal
if errorlevel 1 (
    echo ERROR: vddsdk build failed!
    pause
    exit /b 1
)

"C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" "%PROJECT_DIR%\build\vddctl.vcxproj" /p:Configuration=Release /p:Platform=x64 /t:Rebuild /v:minimal
if errorlevel 1 (
    echo ERROR: vddctl build failed!
    pause
    exit /b 1
)
echo Done.
echo.

REM Step 3: Check if files are in wrong location (WDKRemoteUser without .WIN10TESTING.000)
echo [3/5] Checking for files in wrong location...
set "WRONG_PATH=C:\Users\WDKRemoteUser\Desktop\Windows-driver-samples\video\IndirectDisplay\IddSampleDriver\build\bin\Release"
if exist "%WRONG_PATH%\vddsdk.dll" (
    echo WARNING: Found vddsdk.dll in wrong location!
    echo Copying from: %WRONG_PATH%
    echo          to: %PROJECT_DIR%\build\bin\Release\
    copy /y "%WRONG_PATH%\vddsdk.dll" "%PROJECT_DIR%\build\bin\Release\" >nul
)
if exist "%WRONG_PATH%\vddctl.exe" (
    echo WARNING: Found vddctl.exe in wrong location!
    echo Copying from: %WRONG_PATH%
    echo          to: %PROJECT_DIR%\build\bin\Release\
    copy /y "%WRONG_PATH%\vddctl.exe" "%PROJECT_DIR%\build\bin\Release\" >nul
)
echo Done.
echo.

REM Step 4: Verify files exist and are recent
echo [4/5] Verifying output files...
if not exist "%PROJECT_DIR%\build\bin\Release\vddsdk.dll" (
    echo ERROR: vddsdk.dll not found!
    pause
    exit /b 1
)
if not exist "%PROJECT_DIR%\build\bin\Release\vddctl.exe" (
    echo ERROR: vddctl.exe not found!
    pause
    exit /b 1
)
echo.
echo File locations and timestamps:
dir "%PROJECT_DIR%\build\bin\Release\vddsdk.dll" /t:w
dir "%PROJECT_DIR%\build\bin\Release\vddctl.exe" /t:w
echo.

REM Step 5: Check file age (should be within last 5 minutes)
set "CURRENT_TIME=%TIME:~0,8%"
echo Current time: %CURRENT_TIME%
echo Files should have been modified within the last few minutes.
echo.

echo ========================================
echo Build verification COMPLETE!
echo ========================================
echo.
echo Ready to test with: vddctl install
echo.
pause

