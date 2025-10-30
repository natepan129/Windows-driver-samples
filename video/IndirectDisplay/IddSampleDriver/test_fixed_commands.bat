@echo off
REM 修復後的 VDD 命令測試腳本

echo ========================================
echo VDD 修復後測試
echo ========================================

set VDD_TOOL=.\build\bin\Release\vddctl.exe

echo.
echo 1. 測試版本信息
%VDD_TOOL% version

echo.
echo 2. 初始化 SDK
%VDD_TOOL% init

echo.
echo 3. 檢查狀態（應該顯示系統信息）
%VDD_TOOL% status

echo.
echo 4. 嘗試安裝驅動（應該提示需要先初始化）
%VDD_TOOL% install --path "IddSampleDriver.inf"

echo.
echo 5. 再次檢查狀態
%VDD_TOOL% status

echo.
echo 6. 嘗試激活虛擬顯示（應該提示需要先安裝驅動）
%VDD_TOOL% activate --name "VDD XR" --width 1920 --height 1080 --refresh 90

echo.
echo 7. 關閉 SDK
%VDD_TOOL% shutdown

echo.
echo ========================================
echo 測試完成 - 現在應該有正確的錯誤信息
echo ========================================
pause




