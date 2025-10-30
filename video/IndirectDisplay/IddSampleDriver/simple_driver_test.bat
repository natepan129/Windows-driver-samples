@echo off
REM 簡化的驅動安裝測試

echo ========================================
echo 簡化的驅動安裝測試
echo ========================================

REM 檢查管理員權限
net session >nul 2>&1
if %errorlevel% neq 0 (
    echo 錯誤：需要管理員權限
    echo 請右鍵點擊此文件，選擇"以管理員身份運行"
    pause
    exit /b 1
)

echo ✅ 管理員權限確認

echo.
echo ========================================
echo 測試 1: 直接測試 pnputil 安裝
echo ========================================
echo 命令: pnputil /add-driver "IddSampleDriver_Fixed.inf" /install
pnputil /add-driver "IddSampleDriver_Fixed.inf" /install

echo.
echo ========================================
echo 測試 2: 檢查安裝結果
echo ========================================
echo 列舉已安裝的驅動：
pnputil /enum-drivers | findstr -i "idd"

echo.
echo ========================================
echo 測試 3: 檢查設備狀態
echo ========================================
echo 列舉顯示設備：
pnputil /enum-devices /class Display | findstr -i "idd"

echo.
echo ========================================
echo 測試完成
echo ========================================
pause



