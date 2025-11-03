@echo off
cd /d "%~dp0"
echo ========================================
echo Rebuild CMake Project
echo ========================================
echo Current Directory: %CD%
echo.

echo [1/3] Removing old build directory...
if exist build (
    rmdir /s /q build
    echo Done.
) else (
    echo No build directory found.
)
echo.

echo [2/3] Running CMake to regenerate project files...
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64
if errorlevel 1 (
    echo ERROR: CMake failed!
    pause
    exit /b 1
)
cd ..
echo Done.
echo.

echo [3/3] Building vddctl and vddsdk...
"C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" build\vddsdk.vcxproj /p:Configuration=Release /p:Platform=x64 /v:minimal
"C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" build\vddctl.vcxproj /p:Configuration=Release /p:Platform=x64 /v:minimal
echo.

echo ========================================
echo Checking output files...
echo ========================================
dir build\bin\Release\vddsdk.dll, build\bin\Release\vddctl.exe
echo.

echo ========================================
echo CMake rebuild complete!
echo ========================================
pause

