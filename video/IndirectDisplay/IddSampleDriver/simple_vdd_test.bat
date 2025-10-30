@echo off
REM 簡化的 VDD 功能測試

echo ========================================
echo 簡化的 VDD 功能測試
echo ========================================
echo.

REM 檢查管理員權限
net session >nul 2>&1
if %errorlevel% neq 0 (
    echo 錯誤：需要管理員權限
    pause
    exit /b 1
)

echo ✅ 管理員權限確認

echo.
echo ========================================
echo 測試 1: 檢查驅動安裝狀態
echo ========================================
echo 驅動是否已安裝：
pnputil /enum-drivers | findstr -i "idd"

echo.
echo ========================================
echo 測試 2: 測試 vddctl 基本功能
echo ========================================
echo 版本信息：
build\bin\Release\vddctl.exe version

echo.
echo 初始化 SDK：
build\bin\Release\vddctl.exe init

echo.
echo 檢查狀態：
build\bin\Release\vddctl.exe status

echo.
echo ========================================
echo 測試 3: 嘗試激活虛擬顯示
echo ========================================
echo 激活虛擬顯示：
build\bin\Release\vddctl.exe activate --name "VDD Test" --width 1920 --height 1080 --refresh 60

echo.
echo ========================================
echo 測試完成
echo ========================================
pause



