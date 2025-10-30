@echo off
REM VDD SDK 功能驗證腳本

echo ========================================
echo VDD SDK 功能驗證腳本
echo ========================================
echo.
echo 此腳本將驗證設計文檔中提到的所有功能
echo.

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
echo 驗證 1: 核心 SDK 功能
echo ========================================
echo 檢查文件存在性：
if exist "vddsdk.h" (
    echo ✅ vddsdk.h 存在
) else (
    echo ❌ vddsdk.h 不存在
)

if exist "vddsdk.cpp" (
    echo ✅ vddsdk.cpp 存在
) else (
    echo ❌ vddsdk.cpp 不存在
)

if exist "build\bin\Release\vddctl.exe" (
    echo ✅ vddctl.exe 存在
) else (
    echo ❌ vddctl.exe 不存在
)

echo.
echo ========================================
echo 驗證 2: CLI 工具功能
echo ========================================
echo 測試版本命令：
build\bin\Release\vddctl.exe version

echo.
echo 測試幫助命令：
build\bin\Release\vddctl.exe help

echo.
echo ========================================
echo 驗證 3: 驅動管理功能
echo ========================================
echo 初始化 SDK：
build\bin\Release\vddctl.exe init

echo.
echo 檢查狀態：
build\bin\Release\vddctl.exe status

echo.
echo 檢查驅動安裝：
pnputil /enum-drivers | findstr -i "idd"

echo.
echo ========================================
echo 驗證 4: 虛擬顯示功能
echo ========================================
echo 嘗試激活虛擬顯示：
build\bin\Release\vddctl.exe activate --name "VDD Test" --width 1920 --height 1080 --refresh 60

echo.
echo 檢查顯示器列表：
build\bin\Release\vddctl.exe list

echo.
echo ========================================
echo 驗證 5: 顯示配置功能
echo ========================================
echo 設置顯示模式：
build\bin\Release\vddctl.exe setmode --index 0 --width 1920 --height 1080 --refresh 60

echo.
echo 設置顯示位置：
build\bin\Release\vddctl.exe setlocation --index 0 --x 1920 --y 0 --width 1920 --height 1080

echo.
echo 設置主顯示器：
build\bin\Release\vddctl.exe setprimary --index 0

echo.
echo ========================================
echo 驗證 6: 三種使用場景
echo ========================================
echo 場景 1: VDD 作為顯示代理
echo 命令: vddctl activate --name "VDD Proxy" --width 1920 --height 1080 --refresh 120
build\bin\Release\vddctl.exe activate --name "VDD Proxy" --width 1920 --height 1080 --refresh 120

echo.
echo 場景 2: VDD 作為次要顯示器
echo 命令: vddctl activate --name "VDD Remote" --width 2560 --height 1440 --refresh 90
build\bin\Release\vddctl.exe activate --name "VDD Remote" --width 2560 --height 1440 --refresh 90

echo.
echo 場景 3: VDD 作為主顯示器
echo 命令: vddctl activate --name "VDD Primary" --width 1920 --height 1080 --refresh 90
build\bin\Release\vddctl.exe activate --name "VDD Primary" --width 1920 --height 1080 --refresh 90

echo.
echo ========================================
echo 驗證 7: 清理和關閉
echo ========================================
echo 停用虛擬顯示：
build\bin\Release\vddctl.exe deactivate

echo.
echo 關閉 SDK：
build\bin\Release\vddctl.exe shutdown

echo.
echo ========================================
echo 驗證完成
echo ========================================
echo.
echo 總結：
echo - 如果看到 "Driver Installed: Yes"，說明驅動管理功能正常
echo - 如果虛擬顯示激活成功，說明核心功能正常
echo - 如果所有命令都執行，說明 CLI 工具功能完整
echo - 如果三種場景都演示，說明使用場景支持完整
echo.
pause



