@echo off
REM 基本 VDD 命令測試腳本
REM 測試設計文件中的命令範例

echo ========================================
echo VDD 基本命令測試
echo ========================================

REM 設置環境
set VDD_TOOL=.\build\bin\Release\vddctl.exe
set INF_PATH=IddSampleDriver.inf

echo.
echo 1. 測試版本信息
%VDD_TOOL% version

echo.
echo 2. 測試幫助信息
%VDD_TOOL% help

echo.
echo 3. 初始化 SDK
%VDD_TOOL% init

echo.
echo 4. 檢查狀態
%VDD_TOOL% status

echo.
echo 5. 嘗試安裝驅動（需要管理員權限）
%VDD_TOOL% install --path "%INF_PATH%"

echo.
echo 6. 再次檢查狀態
%VDD_TOOL% status

echo.
echo 7. 嘗試激活虛擬顯示
%VDD_TOOL% activate --name "VDD XR" --width 1920 --height 1080 --refresh 90

echo.
echo 8. 列出顯示器
%VDD_TOOL% list

echo.
echo 9. 關閉 SDK
%VDD_TOOL% shutdown

echo.
echo ========================================
echo 測試完成
echo ========================================
pause




