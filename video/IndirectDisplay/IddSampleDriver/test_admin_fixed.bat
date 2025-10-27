@echo off
REM 修復的管理員權限測試

echo ========================================
echo 修復的管理員權限測試
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
echo 檢查文件存在性
echo ========================================
echo.

echo 檢查 INF 文件：
if exist "IddSampleDriver.inf" (
    echo ✅ IddSampleDriver.inf 存在
) else (
    echo ❌ IddSampleDriver.inf 不存在
)

echo.
echo 檢查 vddctl.exe：
if exist "build\bin\Release\vddctl.exe" (
    echo ✅ vddctl.exe 存在
) else (
    echo ❌ vddctl.exe 不存在
)

echo.
echo ========================================
echo 測試 1: 使用完整路徑初始化 SDK
echo ========================================
echo 命令: build\bin\Release\vddctl.exe init
build\bin\Release\vddctl.exe init

echo.
echo ========================================
echo 測試 2: 使用完整路徑安裝驅動
echo ========================================
echo 命令: build\bin\Release\vddctl.exe install --path "IddSampleDriver.inf"
build\bin\Release\vddctl.exe install --path "IddSampleDriver.inf"

echo.
echo ========================================
echo 測試 3: 使用完整路徑測試 pnputil
echo ========================================
echo 命令: pnputil /add-driver "IddSampleDriver.inf" /install
pnputil /add-driver "IddSampleDriver.inf" /install

echo.
echo ========================================
echo 測試 4: 檢查 INF 文件內容
echo ========================================
echo INF 文件前 10 行：
type IddSampleDriver.inf | more +1 | findstr /n "^" | findstr "^[1-9]:"

echo.
echo ========================================
echo 測試 5: 檢查狀態
echo ========================================
build\bin\Release\vddctl.exe status

echo.
echo ========================================
echo 測試完成
echo ========================================
pause

