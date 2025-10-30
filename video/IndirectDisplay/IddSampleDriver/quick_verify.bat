@echo off
REM 快速驗證腳本 - 測試當前可執行文件

echo ========================================
echo VDD 快速驗證測試
echo ========================================

set VDD_TOOL=.\build\bin\Release\vddctl.exe

echo.
echo 測試 1: 基本功能
%VDD_TOOL% version
echo.

echo 測試 2: 幫助信息
%VDD_TOOL% help | findstr "Commands:"
echo.

echo 測試 3: 初始化
%VDD_TOOL% init
echo.

echo 測試 4: 狀態檢查
%VDD_TOOL% status
echo.

echo 測試 5: 列出顯示器
%VDD_TOOL% list
echo.

echo 測試 6: 關閉
%VDD_TOOL% shutdown
echo.

echo ========================================
echo 驗證完成
echo ========================================
echo.
echo 如果看到 "SDK not initialized" 錯誤，說明需要重新編譯
echo 如果看到系統信息，說明修復已生效
echo.
pause




