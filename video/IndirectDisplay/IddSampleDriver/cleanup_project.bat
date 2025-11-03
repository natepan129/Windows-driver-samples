@echo off
:: Cleanup Test and Temporary Files
:: Keep only production files per design doc

echo ========================================
echo Project Cleanup - Remove Test Files
echo ========================================
echo.
echo This will DELETE:
echo   - 32+ test .bat files
echo   - 14 test .cpp files  
echo   - 24 test .md files
echo   - 7 old .exe files
echo   - IddSampleDriver_Fixed.inf
echo   - Other temporary files
echo.
echo KEEP only:
echo   - vddsdk.dll/lib/h
echo   - vddctl.exe
echo   - Driver source files
echo   - CMakeLists.txt
echo   - IddSampleDriver.vcxproj
echo   - build_vddctl_msbuild.bat
echo.

choice /C YN /M "Continue with cleanup"
if %errorlevel% neq 1 (
    echo Cleanup canceled.
    exit /b 0
)

echo.
echo Starting cleanup...
echo.

:: Delete test .bat files (keep only build_vddctl_msbuild.bat)
echo [1/7] Removing test .bat files...
for %%f in (
    test_*.bat
    quick_*.bat
    run_*.bat
    check_*.bat
    cleanup_*.bat
    debug_*.bat
    diagnose_*.bat
    detailed_*.bat
    force_*.bat
    remove_*.bat
    rebuild_*.bat
    convert_*.ps1
    fix_*.ps1
    setup_*.ps1
    install_*.ps1
    test_*.ps1
) do (
    if exist "%%f" (
        echo   Deleting: %%f
        del /q "%%f" 2>nul
    )
)

:: Delete test .cpp files
echo [2/7] Removing test .cpp files...
for %%f in (test_*.cpp setupapi_*.cpp performance_test.cpp vdd_example.cpp) do (
    if exist "%%f" (
        echo   Deleting: %%f
        del /q "%%f" 2>nul
    )
)

:: Delete test .exe files
echo [3/7] Removing old .exe files...
for %%f in (install_*.exe uninstall_*.exe test_*.exe) do (
    if exist "%%f" (
        echo   Deleting: %%f
        del /q "%%f" 2>nul
    )
)

:: Delete test .md files
echo [4/7] Removing test/documentation .md files...
for %%f in (
    *TEST*.md
    *GUIDE*.md
    *STATUS*.md
    *SUMMARY*.md
    *IMPLEMENTATION*.md
    *VERIFICATION*.md
    *DIAGNOSIS*.md
    *ANALYSIS*.md
    *COMPARISON*.md
    *SOLUTION*.md
    *CHECKLIST*.md
    *REFERENCE*.md
    *COMMANDS*.md
    *ROADMAP*.md
    *PLAN*.md
    *LOG*.md
    *FIX*.md
    *DIFF*.md
    *CHANGES*.md
    *INDEX*.md
    OPTION_*.md
    LEARNED_*.md
    CORRECTED_*.md
    COMMAND_*.md
    DESIGN_FEATURES_*.md
    TRANSLATION_*.md
    FILES_TO_*.md
    QUICK_START_*.md
    TDD_*.md
) do (
    if exist "%%f" (
        echo   Deleting: %%f
        del /q "%%f" 2>nul
    )
)

:: Delete Fixed INF (use MSBuild generated one)
echo [5/7] Removing old INF files...
if exist "IddSampleDriver_Fixed.inf" (
    echo   Deleting: IddSampleDriver_Fixed.inf
    del /q "IddSampleDriver_Fixed.inf" 2>nul
)
if exist "IddSampleDriver.inf" (
    echo   Deleting: IddSampleDriver.inf (root - use x64\Release version)
    del /q "IddSampleDriver.inf" 2>nul
)

:: Delete temporary text files
echo [6/7] Removing temporary files...
for %%f in (*.txt driver_install_backup.txt install_debug.txt uninstall_output.txt) do (
    if exist "%%f" (
        echo   Deleting: %%f
        del /q "%%f" 2>nul
    )
)

:: Delete test_results directory
echo [7/7] Removing test directories...
if exist "test_results" (
    echo   Deleting: test_results\
    rmdir /s /q "test_results" 2>nul
)

echo.
echo ========================================
echo Cleanup Complete!
echo ========================================
echo.
echo Remaining production files:
echo   - vddsdk.cpp/h/def
echo   - vddctl.cpp
echo   - Driver.cpp/h
echo   - CMakeLists.txt
echo   - IddSampleDriver.vcxproj
echo   - build_vddctl_msbuild.bat
echo   - README.md (if kept)
echo   - UPDATED_DESIGN_DOCUMENT.md (optional)
echo.
pause

