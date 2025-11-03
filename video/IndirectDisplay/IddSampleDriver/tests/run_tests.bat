@echo off
REM VDD SDK Test Runner Script
REM Used to run VDD SDK test suite in Windows environment

setlocal enabledelayedexpansion

echo ========================================
echo VDD SDK Test Suite
echo ========================================

REM Set environment variables
set VDD_TEST_MODE=1
set VDD_LOG_LEVEL=debug
set VDD_LOG_FILE=C:\temp\vdd_test.log

REM Create test directories
if not exist "C:\temp" mkdir "C:\temp"
if not exist "C:\temp\vdd_test_data" mkdir "C:\temp\vdd_test_data"
if not exist "C:\temp\vdd_test_reports" mkdir "C:\temp\vdd_test_reports"

REM Check if in correct directory
if not exist "vddsdk_tests.exe" (
    echo ERROR: Cannot find vddsdk_tests.exe
    echo Please run this script from the build directory
    pause
    exit /b 1
)

echo Starting VDD SDK tests...
echo.

REM Run tests
echo Running unit tests...
vddsdk_tests.exe --gtest_filter="*ApiTest*" --gtest_output=xml:C:\temp\vdd_test_reports\unit_tests.xml
if %ERRORLEVEL% neq 0 (
    echo Unit tests failed
    set TEST_FAILED=1
)

echo.
echo Running mock tests...
vddsdk_tests.exe --gtest_filter="*MockTest*" --gtest_output=xml:C:\temp\vdd_test_reports\mock_tests.xml
if %ERRORLEVEL% neq 0 (
    echo Mock tests failed
    set TEST_FAILED=1
)

echo.
echo Running integration tests...
vddsdk_tests.exe --gtest_filter="*IntegrationTest*" --gtest_output=xml:C:\temp\vdd_test_reports\integration_tests.xml
if %ERRORLEVEL% neq 0 (
    echo Integration tests failed
    set TEST_FAILED=1
)

echo.
echo Running performance tests...
vddsdk_tests.exe --gtest_filter="*Performance*" --gtest_output=xml:C:\temp\vdd_test_reports\performance_tests.xml
if %ERRORLEVEL% neq 0 (
    echo Performance tests failed
    set TEST_FAILED=1
)

echo.
echo Running all tests...
vddsdk_tests.exe --gtest_output=xml:C:\temp\vdd_test_reports\all_tests.xml
if %ERRORLEVEL% neq 0 (
    echo Some tests failed
    set TEST_FAILED=1
)

REM Generate test report
echo.
echo Generating test reports...
if exist "C:\temp\vdd_test_reports\all_tests.xml" (
    echo Test report generated: C:\temp\vdd_test_reports\all_tests.xml
)

REM Cleanup test environment
echo.
echo Cleaning up test environment...
if exist "C:\temp\vdd_test_data" rmdir /s /q "C:\temp\vdd_test_data"

REM Display results
echo.
echo ========================================
if defined TEST_FAILED (
    echo Test Result: FAILED
    echo Please check test reports for details
) else (
    echo Test Result: PASSED
    echo All tests passed
)
echo ========================================

REM Wait for user confirmation
pause
exit /b %ERRORLEVEL%
