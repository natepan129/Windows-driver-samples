@echo off
echo VDD SDK 建置和測試腳本
echo ========================

REM 設定環境變數
set CMAKE_BUILD_TYPE=Debug
set BUILD_DIR=build
set TEST_RESULTS_DIR=test_results

REM 清理舊的建置
if exist %BUILD_DIR% (
    echo 清理舊的建置目錄...
    rmdir /s /q %BUILD_DIR%
)

if exist %TEST_RESULTS_DIR% (
    echo 清理舊的測試結果...
    rmdir /s /q %TEST_RESULTS_DIR%
)

REM 創建建置目錄
mkdir %BUILD_DIR%
mkdir %TEST_RESULTS_DIR%

REM 進入建置目錄
cd %BUILD_DIR%

echo.
echo 步驟 1: 配置 CMake...
cmake .. -G "Visual Studio 17 2022" -A x64
if %ERRORLEVEL% neq 0 (
    echo ❌ CMake 配置失敗
    exit /b 1
)

echo.
echo 步驟 2: 建置專案...
cmake --build . --config Debug
if %ERRORLEVEL% neq 0 (
    echo ❌ 建置失敗
    exit /b 1
)

echo.
echo 步驟 3: 執行測試...
echo 執行基本功能測試...
bin\Debug\test_basic_functionality.exe > ..\%TEST_RESULTS_DIR%\basic_functionality.log 2>&1
if %ERRORLEVEL% neq 0 (
    echo ❌ 基本功能測試失敗
    type ..\%TEST_RESULTS_DIR%\basic_functionality.log
    exit /b 1
) else (
    echo ✓ 基本功能測試通過
)

echo 執行驅動程式 IOCTL 測試...
bin\Debug\test_driver_ioctl.exe > ..\%TEST_RESULTS_DIR%\driver_ioctl.log 2>&1
if %ERRORLEVEL% neq 0 (
    echo ❌ 驅動程式 IOCTL 測試失敗
    type ..\%TEST_RESULTS_DIR%\driver_ioctl.log
    exit /b 1
) else (
    echo ✓ 驅動程式 IOCTL 測試通過
)

echo 執行 VDD 測試...
bin\Debug\test_vdd.exe > ..\%TEST_RESULTS_DIR%\vdd.log 2>&1
if %ERRORLEVEL% neq 0 (
    echo ❌ VDD 測試失敗
    type ..\%TEST_RESULTS_DIR%\vdd.log
    exit /b 1
) else (
    echo ✓ VDD 測試通過
)

echo.
echo 步驟 4: 生成測試報告...
echo 測試報告 > ..\%TEST_RESULTS_DIR%\report.txt
echo ========== >> ..\%TEST_RESULTS_DIR%\report.txt
echo. >> ..\%TEST_RESULTS_DIR%\report.txt
echo 建置時間: %DATE% %TIME% >> ..\%TEST_RESULTS_DIR%\report.txt
echo. >> ..\%TEST_RESULTS_DIR%\report.txt

echo 基本功能測試結果: >> ..\%TEST_RESULTS_DIR%\report.txt
type ..\%TEST_RESULTS_DIR%\basic_functionality.log >> ..\%TEST_RESULTS_DIR%\report.txt
echo. >> ..\%TEST_RESULTS_DIR%\report.txt

echo 驅動程式 IOCTL 測試結果: >> ..\%TEST_RESULTS_DIR%\report.txt
type ..\%TEST_RESULTS_DIR%\driver_ioctl.log >> ..\%TEST_RESULTS_DIR%\report.txt
echo. >> ..\%TEST_RESULTS_DIR%\report.txt

echo VDD 測試結果: >> ..\%TEST_RESULTS_DIR%\report.txt
type ..\%TEST_RESULTS_DIR%\vdd.log >> ..\%TEST_RESULTS_DIR%\report.txt

echo.
echo 🎉 所有測試完成！
echo 測試結果已儲存到 %TEST_RESULTS_DIR% 目錄
echo.
echo 建置的檔案:
dir bin\Debug\*.exe
dir lib\Debug\*.dll
dir lib\Debug\*.lib

cd ..
echo.
echo 建置和測試完成！
