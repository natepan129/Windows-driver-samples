@echo off
REM 正確安裝 IddSampleDriver 到 ROOT 位置

echo ========================================
echo 正確安裝 IddSampleDriver 到 ROOT 位置
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
echo 步驟 1: 移除現有的錯誤安裝
echo ========================================
echo 移除 SWD\IddSampleDriver 設備...
pnputil /remove-device "SWD\IddSampleDriver\IddSampleDriver"

echo.
echo ========================================
echo 步驟 2: 使用正確的硬體 ID 安裝
echo ========================================
echo 安裝到 ROOT\IddSampleDriver...
pnputil /add-driver "IddSampleDriver.inf" /install

echo.
echo ========================================
echo 步驟 3: 檢查安裝結果
echo ========================================
echo 檢查 ROOT 設備：
pnputil /enum-devices | findstr -i "root.*idd"

echo.
echo 檢查所有 IddSampleDriver 設備：
pnputil /enum-devices | findstr -i "idd"

echo.
echo ========================================
echo 步驟 4: 測試 SDK
echo ========================================
build\bin\Release\vddctl.exe init
build\bin\Release\vddctl.exe status

echo.
echo ========================================
echo 完成
echo ========================================
pause



