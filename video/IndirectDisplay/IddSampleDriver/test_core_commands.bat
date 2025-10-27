@echo off
REM 測試所有驅動安裝核心命令

echo ========================================
echo 測試驅動安裝核心命令
echo ========================================
echo.
echo 這個測試將驗證所有核心驅動安裝方法：
echo 1. pnputil 命令（最可靠）
echo 2. UpdateDriverForPlugAndPlayDevices API
echo 3. SetupAPI 方法
echo.

set VDD_TOOL=.\build\bin\Release\vddctl.exe

echo.
echo ========================================
echo 測試 1: 檢查 pnputil 是否可用
echo ========================================
pnputil /?
if %errorlevel% == 0 (
    echo ✅ pnputil 可用
) else (
    echo ❌ pnputil 不可用
)

echo.
echo ========================================
echo 測試 2: 初始化 SDK
echo ========================================
%VDD_TOOL% init

echo.
echo ========================================
echo 測試 3: 嘗試安裝驅動（使用所有方法）
echo ========================================
echo 命令: vddctl install --path "IddSampleDriver.inf"
%VDD_TOOL% install --path "IddSampleDriver.inf"

echo.
echo ========================================
echo 測試 4: 檢查安裝結果
echo ========================================
%VDD_TOOL% status

echo.
echo ========================================
echo 測試 5: 手動測試 pnputil 命令
echo ========================================
echo 命令: pnputil /add-driver "IddSampleDriver.inf" /install
pnputil /add-driver "IddSampleDriver.inf" /install

echo.
echo ========================================
echo 測試完成
echo ========================================
echo.
echo 如果看到 "Driver installed successfully via pnputil"，說明核心命令正常
echo 如果看到 "All installation methods failed"，說明需要檢查權限或 INF 文件
echo.
pause

