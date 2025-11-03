@echo off
echo ==========================================
echo Rebuild CMake in Correct Path
echo ==========================================
echo.

echo Current directory:
cd

echo.
echo Step 1: Remove old build directory from wrong path...
if exist "C:\Users\WDKRemoteUser\Desktop\Windows-driver-samples\video\IndirectDisplay\IddSampleDriver\build" (
    rmdir /S /Q "C:\Users\WDKRemoteUser\Desktop\Windows-driver-samples\video\IndirectDisplay\IddSampleDriver\build"
    echo   Old build deleted
) else (
    echo   Old build not found
)

echo.
echo Step 2: Remove local build directory if exists...
if exist "build" (
    rmdir /S /Q "build"
    echo   Local build deleted
)

echo.
echo Step 3: Run CMake to generate project files...
cmake -B build -G "Visual Studio 17 2022" -A x64

echo.
echo Step 4: Verify CMakeCache.txt path...
findstr "For build in directory" build\CMakeCache.txt

echo.
echo Step 5: Build with MSBuild...
"C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" build\VddSdk.sln /p:Configuration=Release /p:Platform=x64 /v:minimal

echo.
echo Step 6: Verify output files...
dir build\bin\Release\vddsdk.dll
dir build\bin\Release\vddctl.exe

echo.
echo ==========================================
echo Done! Files should now be in correct location
echo ==========================================
pause

