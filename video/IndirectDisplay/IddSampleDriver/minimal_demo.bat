@echo off
REM 最小可工作演示 - 不依賴驅動安裝

echo ========================================
echo VDD 最小可工作演示
echo ========================================

set VDD_TOOL=.\build\bin\Release\vddctl.exe

echo.
echo 1. 測試基本功能（不需要驅動）
%VDD_TOOL% version
%VDD_TOOL% help

echo.
echo 2. 初始化 SDK
%VDD_TOOL% init

echo.
echo 3. 檢查狀態
%VDD_TOOL% status

echo.
echo 4. 列出顯示器（模擬模式）
%VDD_TOOL% list

echo.
echo 5. 關閉 SDK
%VDD_TOOL% shutdown

echo.
echo ========================================
echo 演示完成 - 基本功能正常
echo ========================================
pause




