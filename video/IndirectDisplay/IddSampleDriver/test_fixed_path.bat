@echo off
REM 修復路徑問題的 VDD 測試

echo ========================================
echo 修復路徑問題的 VDD 測試
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

REM 設置正確的工作目錄
cd /d "%~dp0"
echo 當前工作目錄: %CD%

echo.
echo ========================================
echo 測試 1: 檢查文件存在性
echo ========================================
if exist "build\bin\Release\vddctl.exe" (
    echo ✅ vddctl.exe 存在
) else (
    echo ❌ vddctl.exe 不存在
    pause
    exit /b 1
)

if exist "IddSampleDriver_Fixed.inf" (
    echo ✅ 修復的 INF 文件存在
) else (
    echo ❌ 修復的 INF 文件不存在
)

echo.
echo ========================================
echo 測試 2: 檢查驅動安裝狀態
echo ========================================
echo 檢查驅動是否在驅動存儲中：
pnputil /enum-drivers | findstr -i "idd"

echo.
echo ========================================
echo 測試 3: 使用完整路徑初始化 SDK
echo ========================================
echo 命令: %CD%\build\bin\Release\vddctl.exe init
"%CD%\build\bin\Release\vddctl.exe" init

echo.
echo ========================================
echo 測試 4: 檢查 SDK 狀態
echo ========================================
"%CD%\build\bin\Release\vddctl.exe" status

echo.
echo ========================================
echo 測試 5: 嘗試激活虛擬顯示
echo ========================================
echo 命令: vddctl activate --name "VDD Test" --width 1920 --height 1080 --refresh 60
"%CD%\build\bin\Release\vddctl.exe" activate --name "VDD Test" --width 1920 --height 1080 --refresh 60

echo.
echo ========================================
echo 測試 6: 檢查顯示器列表
echo ========================================
"%CD%\build\bin\Release\vddctl.exe" list

echo.
echo ========================================
echo 測試完成
echo ========================================
pause



