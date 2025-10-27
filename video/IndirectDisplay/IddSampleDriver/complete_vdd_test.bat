@echo off
REM 完整的 VDD 功能測試

echo ========================================
echo 完整的 VDD 功能測試
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
echo 檢查驅動是否在驅動存儲中：
pnputil /enum-drivers | findstr -i "idd"

echo.
echo ========================================
echo 測試 2: 初始化 SDK
echo ========================================
build\bin\Release\vddctl.exe init

echo.
echo ========================================
echo 測試 3: 檢查 SDK 狀態
echo ========================================
build\bin\Release\vddctl.exe status

echo.
echo ========================================
echo 測試 4: 嘗試激活虛擬顯示
echo ========================================
echo 命令: vddctl activate --name "VDD Test" --width 1920 --height 1080 --refresh 60
build\bin\Release\vddctl.exe activate --name "VDD Test" --width 1920 --height 1080 --refresh 60

echo.
echo ========================================
echo 測試 5: 檢查顯示器列表
echo ========================================
build\bin\Release\vddctl.exe list

echo.
echo ========================================
echo 測試 6: 設置顯示模式
echo ========================================
echo 命令: vddctl setmode --index 0 --width 1920 --height 1080 --refresh 60
build\bin\Release\vddctl.exe setmode --index 0 --width 1920 --height 1080 --refresh 60

echo.
echo ========================================
echo 測試 7: 停用虛擬顯示
echo ========================================
build\bin\Release\vddctl.exe deactivate

echo.
echo ========================================
echo 測試 8: 關閉 SDK
echo ========================================
build\bin\Release\vddctl.exe shutdown

echo.
echo ========================================
echo 測試完成
echo ========================================
echo.
echo 如果看到 "Driver Installed: Yes"，說明修復成功！
echo 如果虛擬顯示激活成功，說明 VDD 完全工作！
echo.
pause
