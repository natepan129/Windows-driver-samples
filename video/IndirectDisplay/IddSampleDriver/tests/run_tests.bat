@echo off
REM VDD SDK 測試運行腳本
REM 用於在 Windows 環境中運行 VDD SDK 測試套件

setlocal enabledelayedexpansion

echo ========================================
echo VDD SDK 測試套件
echo ========================================

REM 設置環境變量
set VDD_TEST_MODE=1
set VDD_LOG_LEVEL=debug
set VDD_LOG_FILE=C:\temp\vdd_test.log

REM 創建測試目錄
if not exist "C:\temp" mkdir "C:\temp"
if not exist "C:\temp\vdd_test_data" mkdir "C:\temp\vdd_test_data"
if not exist "C:\temp\vdd_test_reports" mkdir "C:\temp\vdd_test_reports"

REM 檢查是否在正確的目錄中
if not exist "vddsdk_tests.exe" (
    echo 錯誤: 找不到 vddsdk_tests.exe
    echo 請確保在構建目錄中運行此腳本
    pause
    exit /b 1
)

echo 開始運行 VDD SDK 測試...
echo.

REM 運行測試
echo 運行單元測試...
vddsdk_tests.exe --gtest_filter="*ApiTest*" --gtest_output=xml:C:\temp\vdd_test_reports\unit_tests.xml
if %ERRORLEVEL% neq 0 (
    echo 單元測試失敗
    set TEST_FAILED=1
)

echo.
echo 運行模擬測試...
vddsdk_tests.exe --gtest_filter="*MockTest*" --gtest_output=xml:C:\temp\vdd_test_reports\mock_tests.xml
if %ERRORLEVEL% neq 0 (
    echo 模擬測試失敗
    set TEST_FAILED=1
)

echo.
echo 運行集成測試...
vddsdk_tests.exe --gtest_filter="*IntegrationTest*" --gtest_output=xml:C:\temp\vdd_test_reports\integration_tests.xml
if %ERRORLEVEL% neq 0 (
    echo 集成測試失敗
    set TEST_FAILED=1
)

echo.
echo 運行性能測試...
vddsdk_tests.exe --gtest_filter="*Performance*" --gtest_output=xml:C:\temp\vdd_test_reports\performance_tests.xml
if %ERRORLEVEL% neq 0 (
    echo 性能測試失敗
    set TEST_FAILED=1
)

echo.
echo 運行所有測試...
vddsdk_tests.exe --gtest_output=xml:C:\temp\vdd_test_reports\all_tests.xml
if %ERRORLEVEL% neq 0 (
    echo 部分測試失敗
    set TEST_FAILED=1
)

REM 生成測試報告
echo.
echo 生成測試報告...
if exist "C:\temp\vdd_test_reports\all_tests.xml" (
    echo 測試報告已生成: C:\temp\vdd_test_reports\all_tests.xml
)

REM 清理測試環境
echo.
echo 清理測試環境...
if exist "C:\temp\vdd_test_data" rmdir /s /q "C:\temp\vdd_test_data"

REM 顯示結果
echo.
echo ========================================
if defined TEST_FAILED (
    echo 測試結果: 失敗
    echo 請檢查測試報告以獲取詳細信息
) else (
    echo 測試結果: 成功
    echo 所有測試通過
)
echo ========================================

REM 等待用戶確認
pause
exit /b %ERRORLEVEL%
