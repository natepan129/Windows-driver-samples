@echo off
REM VDD SDK 第一個 TDD 測試構建和運行腳本

setlocal enabledelayedexpansion

echo ========================================
echo VDD SDK 第一個 TDD 測試
echo ========================================

REM 設置環境變量
set VDD_TEST_MODE=1
set VDD_LOG_LEVEL=debug
set VDD_LOG_FILE=C:\temp\vdd_first_test.log

REM 創建必要的目錄
if not exist "build" mkdir "build"
if not exist "C:\temp" mkdir "C:\temp"

echo 1. 構建 VDD SDK...
cd build

REM 配置 CMake
echo 配置 CMake...
cmake .. -DBUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Release
if %ERRORLEVEL% neq 0 (
    echo CMake 配置失敗
    pause
    exit /b 1
)

REM 構建項目
echo 構建項目...
cmake --build . --config Release
if %ERRORLEVEL% neq 0 (
    echo 構建失敗
    pause
    exit /b 1
)

echo 構建成功！
echo.

REM 運行第一個 TDD 測試
echo 2. 運行第一個 TDD 測試...
echo.

if exist "Release\first_tdd_test.exe" (
    echo 運行 first_tdd_test.exe...
    Release\first_tdd_test.exe
    if %ERRORLEVEL% neq 0 (
        echo 第一個 TDD 測試失敗
        set TEST_FAILED=1
    ) else (
        echo 第一個 TDD 測試通過！
    )
) else (
    echo 錯誤: 找不到 first_tdd_test.exe
    echo 請檢查構建是否成功
    set TEST_FAILED=1
)

echo.

REM 運行完整的測試套件
echo 3. 運行完整測試套件...
echo.

if exist "Release\vddsdk_tests.exe" (
    echo 運行 vddsdk_tests.exe...
    Release\vddsdk_tests.exe --gtest_filter="*FirstTddTest*"
    if %ERRORLEVEL% neq 0 (
        echo 完整測試套件失敗
        set TEST_FAILED=1
    ) else (
        echo 完整測試套件通過！
    )
) else (
    echo 錯誤: 找不到 vddsdk_tests.exe
    echo 請檢查構建是否成功
    set TEST_FAILED=1
)

echo.

REM 顯示結果
echo ========================================
if defined TEST_FAILED (
    echo 測試結果: 失敗
    echo 請檢查錯誤信息
) else (
    echo 測試結果: 成功
    echo 第一個 TDD 測試完成！
)
echo ========================================

REM 等待用戶確認
pause
exit /b %ERRORLEVEL%
