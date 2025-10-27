@echo off
REM 測試修復後的 UMDF 安裝

echo ========================================
echo 測試修復後的 UMDF 安裝
echo ========================================

set VDD_TOOL=.\build\bin\Release\vddctl.exe

echo.
echo 1. 初始化 SDK
%VDD_TOOL% init

echo.
echo 2. 檢查當前狀態
%VDD_TOOL% status

echo.
echo 3. 嘗試安裝 UMDF 驅動（需要管理員權限）
echo 注意：這將使用正確的 UMDF 安裝方法
%VDD_TOOL% install --path "IddSampleDriver.inf"

echo.
echo 4. 再次檢查狀態
%VDD_TOOL% status

echo.
echo 5. 如果安裝成功，嘗試激活虛擬顯示
%VDD_TOOL% activate --name "VDD XR" --width 1920 --height 1080 --refresh 90

echo.
echo ========================================
echo 測試完成
echo ========================================
echo.
echo 如果看到 "UMDF driver installed successfully"，說明修復成功
echo 如果仍然失敗，可能需要重新編譯或檢查權限
echo.
pause
