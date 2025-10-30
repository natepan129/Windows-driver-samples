@echo off
REM ============================================
REM VDD SDK Release Build Test Script
REM ============================================

echo.
echo ========================================
echo Step 1: Clean previous build
echo ========================================
msbuild IddSampleDriver.sln /t:Clean /p:Configuration=Release /p:Platform=x64
if errorlevel 1 (
    echo [FAILED] Clean failed
    exit /b 1
)
echo [OK] Clean completed

echo.
echo ========================================
echo Step 2: Build Release configuration
echo ========================================
msbuild IddSampleDriver.sln /p:Configuration=Release /p:Platform=x64 /v:minimal
if errorlevel 1 (
    echo [FAILED] Build failed
    exit /b 1
)
echo [OK] Build completed

echo.
echo ========================================
echo Step 3: Verify build outputs
echo ========================================
set RELEASE_DIR=x64\Release

if not exist "%RELEASE_DIR%\IddSampleDriver.dll" (
    echo [FAILED] IddSampleDriver.dll not found
    exit /b 1
)
echo [OK] IddSampleDriver.dll exists

if not exist "%RELEASE_DIR%\IddSampleDriver.inf" (
    echo [FAILED] IddSampleDriver.inf not found
    exit /b 1
)
echo [OK] IddSampleDriver.inf exists

if not exist "%RELEASE_DIR%\vddsdk.dll" (
    echo [WARNING] vddsdk.dll not found (may not be built yet)
) else (
    echo [OK] vddsdk.dll exists
)

if not exist "%RELEASE_DIR%\vddctl.exe" (
    echo [WARNING] vddctl.exe not found (may not be built yet)
) else (
    echo [OK] vddctl.exe exists
)

echo.
echo ========================================
echo Step 4: Check for test tools
echo ========================================
if exist "install_driver.exe" (
    echo [OK] install_driver.exe available
) else (
    echo [INFO] install_driver.exe not found, compile it with:
    echo        cl /EHsc setupapi_install_with_rollback.cpp setupapi.lib newdev.lib cfgmgr32.lib advapi32.lib /Fe:install_driver.exe
)

if exist "uninstall_driver.exe" (
    echo [OK] uninstall_driver.exe available
) else (
    echo [INFO] uninstall_driver.exe not found, compile it with:
    echo        cl /EHsc setupapi_uninstall.cpp setupapi.lib newdev.lib cfgmgr32.lib advapi32.lib /Fe:uninstall_driver.exe
)

echo.
echo ========================================
echo Build Summary
echo ========================================
echo Release build SUCCESSFUL!
echo.
echo Test installation with:
echo   install_driver.exe %RELEASE_DIR%\IddSampleDriver.inf
echo.
echo Or manually test:
echo   1. Open Device Manager
echo   2. Right-click and "Add legacy hardware"
echo   3. Browse to %RELEASE_DIR%\IddSampleDriver.inf
echo.
echo ========================================

exit /b 0

