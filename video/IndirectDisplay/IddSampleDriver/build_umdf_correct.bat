@echo off
REM Build UMDF Driver to Correct Path

echo ========================================
echo Build UMDF Driver Correctly
echo ========================================

REM Check Administrator Privileges
net session >nul 2>&1
if %errorlevel% neq 0 (
    echo ERROR: Administrator privileges required
    echo Please right-click this file and select "Run as administrator"
    pause
    exit /b 1
)

echo SUCCESS: Administrator privileges confirmed

echo.
echo ========================================
echo Step 1: Check Visual Studio Environment
echo ========================================
echo Checking MSBuild...
where msbuild
if %errorlevel% neq 0 (
    echo ERROR: MSBuild not found
    echo Please install Visual Studio or Build Tools
    pause
    exit /b 1
)

echo SUCCESS: MSBuild found

echo.
echo ========================================
echo Step 2: Clean Old Builds
echo ========================================
echo Cleaning Debug build...
if exist "x64\Debug" rmdir /s /q "x64\Debug"
if exist "x64\Release" rmdir /s /q "x64\Release"

echo.
echo ========================================
echo Step 3: Build Debug Version
echo ========================================
echo Building Debug version...
msbuild IddSampleDriver.vcxproj /p:Configuration=Debug /p:Platform=x64 /p:TargetVersion=Windows10 /v:minimal

if %errorlevel% neq 0 (
    echo ERROR: Debug build failed
    echo Please check error messages
    pause
    exit /b 1
)

echo SUCCESS: Debug build completed

echo.
echo ========================================
echo Step 4: Build Release Version
echo ========================================
echo Building Release version...
msbuild IddSampleDriver.vcxproj /p:Configuration=Release /p:Platform=x64 /p:TargetVersion=Windows10 /v:minimal

if %errorlevel% neq 0 (
    echo ERROR: Release build failed
    echo Please check error messages
    pause
    exit /b 1
)

echo SUCCESS: Release build completed

echo.
echo ========================================
echo Step 5: Check Build Results
echo ========================================
echo Checking Debug build results...
dir "x64\Debug\IddSampleDriver" 2>nul
if %errorlevel% neq 0 (
    echo ERROR: Debug build directory not found
    echo Checking Debug directory...
    dir x64\Debug\
) else (
    echo SUCCESS: Debug build directory found
    dir "x64\Debug\IddSampleDriver"
)

echo.
echo Checking Release build results...
dir "x64\Release\IddSampleDriver" 2>nul
if %errorlevel% neq 0 (
    echo ERROR: Release build directory not found
    echo Checking Release directory...
    dir x64\Release\
) else (
    echo SUCCESS: Release build directory found
    dir "x64\Release\IddSampleDriver"
)

echo.
echo ========================================
echo Step 6: Install Driver using Debug Version
echo ========================================
echo Installing using Debug version...
if exist "x64\Debug\IddSampleDriver\IddSampleDriver.inf" (
    echo Installing Debug version driver...
    pnputil /add-driver "x64\Debug\IddSampleDriver\IddSampleDriver.inf" /install
) else (
    echo ERROR: Debug INF file not found
)

echo.
echo ========================================
echo Step 7: Check Installation Results
echo ========================================
echo Checking ROOT devices:
pnputil /enum-devices | findstr -i "root.*idd"

echo.
echo Checking all IddSampleDriver devices:
pnputil /enum-devices | findstr -i "idd"

echo.
echo ========================================
echo COMPLETED
echo ========================================
pause
