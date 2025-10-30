@echo off
REM 啟用 IddSampleDriver 設備

echo ========================================
echo 啟用 IddSampleDriver 設備
echo ========================================

REM 檢查管理員權限
net session >nul 2>&1
if %errorlevel% neq 0 (
    echo ❌ 需要管理員權限
    echo 請右鍵點擊此文件，選擇"以管理員身份運行"
    pause
    exit /b 1
)

echo ✅ 管理員權限確認

echo.
echo ========================================
echo 檢查設備狀態
echo ========================================
pnputil /enum-devices /class Display | findstr IddSampleDriver

echo.
echo ========================================
echo 嘗試啟用設備
echo ========================================
echo 啟用 IddSampleDriver 設備...
pnputil /enable-device "SWD\IddSampleDriver\IddSampleDriver"

echo.
echo ========================================
echo 檢查啟用結果
echo ========================================
pnputil /enum-devices /class Display | findstr IddSampleDriver

echo.
echo ========================================
echo 測試 SDK 狀態
echo ========================================
build\bin\Release\vddctl.exe init
build\bin\Release\vddctl.exe status

echo.
echo ========================================
echo 完成
echo ========================================
pause



