@echo off
REM 使用修復 INF 文件的測試

echo ========================================
echo 使用修復 INF 文件的測試
echo ========================================
echo.

REM 檢查是否為管理員
net session >nul 2>&1
if %errorlevel% == 0 (
    echo ✅ 已獲得管理員權限
) else (
    echo ❌ 需要管理員權限
    pause
    exit /b 1
)

echo.
echo ========================================
echo 測試 1: 檢查修復的 INF 文件
echo ========================================
if exist "IddSampleDriver_Fixed.inf" (
    echo ✅ 修復的 INF 文件存在
) else (
    echo ❌ 修復的 INF 文件不存在
    pause
    exit /b 1
)

echo.
echo ========================================
echo 測試 2: 使用修復的 INF 文件測試 pnputil
echo ========================================
echo 命令: pnputil /add-driver "IddSampleDriver_Fixed.inf" /install
pnputil /add-driver "IddSampleDriver_Fixed.inf" /install

echo.
echo ========================================
echo 測試 3: 檢查驅動是否安裝成功
echo ========================================
pnputil /enum-drivers | findstr "IddSampleDriver"

echo.
echo ========================================
echo 測試 4: 使用 vddctl 檢查狀態
echo ========================================
build\bin\Release\vddctl.exe init
build\bin\Release\vddctl.exe status

echo.
echo ========================================
echo 測試完成
echo ========================================
echo.
echo 如果看到 "Driver installed successfully via pnputil"，說明修復成功！
echo.
pause

