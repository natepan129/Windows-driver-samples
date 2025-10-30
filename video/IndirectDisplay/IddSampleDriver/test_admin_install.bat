@echo off
REM 以管理員身份測試驅動安裝

echo ========================================
echo 管理員權限測試
echo ========================================
echo.
echo 注意：此腳本需要以管理員身份運行
echo 請右鍵點擊此文件，選擇"以管理員身份運行"
echo.

REM 檢查是否為管理員
net session >nul 2>&1
if %errorlevel% == 0 (
    echo ✅ 已獲得管理員權限
) else (
    echo ❌ 需要管理員權限
    echo 請右鍵點擊此文件，選擇"以管理員身份運行"
    pause
    exit /b 1
)

set VDD_TOOL=.\build\bin\Release\vddctl.exe

echo.
echo ========================================
echo 測試 1: 初始化 SDK
echo ========================================
%VDD_TOOL% init

echo.
echo ========================================
echo 測試 2: 嘗試安裝驅動（管理員權限）
echo ========================================
%VDD_TOOL% install --path "IddSampleDriver.inf"

echo.
echo ========================================
echo 測試 3: 手動測試 pnputil（管理員權限）
echo ========================================
pnputil /add-driver "IddSampleDriver.inf" /install

echo.
echo ========================================
echo 測試 4: 檢查安裝結果
echo ========================================
%VDD_TOOL% status

echo.
echo ========================================
echo 測試完成
echo ========================================
pause




