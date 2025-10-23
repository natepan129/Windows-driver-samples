@echo off
REM VDD 完整部署腳本
REM 需要以管理員身份執行

echo ========================================
echo VDD (Virtual Display Driver) 部署腳本
echo ========================================

REM 檢查管理員權限
net session >nul 2>&1
if %errorLevel% == 0 (
    echo [INFO] 管理員權限確認
) else (
    echo [ERROR] 需要管理員權限，請以管理員身份執行此腳本
    pause
    exit /b 1
)

REM 設定路徑
set VDD_DIR=%~dp0
set DRIVER_DIR=%VDD_DIR%x64\Debug\IddSampleDriver
set CLI_DIR=%VDD_DIR%x64\Debug

echo [INFO] VDD 目錄: %VDD_DIR%
echo [INFO] 驅動程式目錄: %DRIVER_DIR%
echo [INFO] CLI 目錄: %CLI_DIR%

REM 檢查必要檔案
if not exist "%DRIVER_DIR%\IddSampleDriver.dll" (
    echo [ERROR] 找不到驅動程式檔案: %DRIVER_DIR%\IddSampleDriver.dll
    echo [INFO] 請先建置專案
    pause
    exit /b 1
)

if not exist "%DRIVER_DIR%\IddSampleDriver.inf" (
    echo [ERROR] 找不到驅動程式 INF 檔案: %DRIVER_DIR%\IddSampleDriver.inf
    pause
    exit /b 1
)

echo [INFO] 必要檔案檢查完成

REM 步驟 1: 安裝驅動程式
echo.
echo ========================================
echo 步驟 1: 安裝 VDD 驅動程式
echo ========================================

echo [INFO] 安裝驅動程式...
pnputil /add-driver "%DRIVER_DIR%\IddSampleDriver.inf" /install
if %errorLevel% == 0 (
    echo [SUCCESS] 驅動程式安裝成功
) else (
    echo [WARNING] 驅動程式安裝可能失敗，繼續執行...
)

REM 步驟 2: 初始化 VDD SDK
echo.
echo ========================================
echo 步驟 2: 初始化 VDD SDK
echo ========================================

echo [INFO] 初始化 VDD SDK...
REM 注意: 這裡需要編譯 vddctl.exe
REM 如果沒有 vddctl.exe，請先建置專案
if exist "%CLI_DIR%\vddctl.exe" (
    "%CLI_DIR%\vddctl.exe" init
    if %errorLevel% == 0 (
        echo [SUCCESS] VDD SDK 初始化成功
    ) else (
        echo [WARNING] VDD SDK 初始化失敗
    )
) else (
    echo [WARNING] 找不到 vddctl.exe，請先建置專案
    echo [INFO] 使用 Visual Studio 建置 IddSampleDriver.sln
)

REM 步驟 3: 啟用虛擬顯示器
echo.
echo ========================================
echo 步驟 3: 啟用虛擬顯示器
echo ========================================

if exist "%CLI_DIR%\vddctl.exe" (
    echo [INFO] 啟用虛擬顯示器...
    "%CLI_DIR%\vddctl.exe" activate --name "VDD XR Display" --width 1920 --height 1080 --refresh 90 --count 1
    
    echo [INFO] 設定顯示器位置...
    "%CLI_DIR%\vddctl.exe" setlocation --index 0 --x 3840 --y 0 --width 1920 --height 1080
    
    echo [INFO] 檢查狀態...
    "%CLI_DIR%\vddctl.exe" status
    
    echo [INFO] 列出所有顯示器...
    "%CLI_DIR%\vddctl.exe" list
) else (
    echo [WARNING] 無法執行 VDD 命令，請先建置 vddctl.exe
)

echo.
echo ========================================
echo 部署完成
echo ========================================
echo.
echo 使用說明:
echo 1. 檢查虛擬顯示器是否出現在 Windows 顯示設定中
echo 2. 使用 vddctl.exe 管理虛擬顯示器
echo 3. 執行 vddctl.exe help 查看所有可用命令
echo.
echo 常用命令:
echo   vddctl status          - 檢查狀態
echo   vddctl list            - 列出顯示器
echo   vddctl deactivate      - 停用虛擬顯示器
echo   vddctl shutdown         - 關閉 SDK
echo.

pause
