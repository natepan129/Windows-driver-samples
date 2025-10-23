@echo off
REM 簡單 VDD 部署腳本 (使用現有檔案)
echo ========================================
echo VDD 簡單部署腳本
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

echo [INFO] VDD 目錄: %VDD_DIR%
echo [INFO] 驅動程式目錄: %DRIVER_DIR%

REM 檢查必要檔案
if not exist "%DRIVER_DIR%\IddSampleDriver.dll" (
    echo [ERROR] 找不到驅動程式檔案: %DRIVER_DIR%\IddSampleDriver.dll
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

REM 步驟 2: 檢查驅動程式狀態
echo.
echo ========================================
echo 步驟 2: 檢查驅動程式狀態
echo ========================================

echo [INFO] 檢查驅動程式是否已安裝...
pnputil /enum-drivers | findstr "IddSampleDriver" >nul
if %errorLevel% == 0 (
    echo [SUCCESS] 驅動程式已安裝
) else (
    echo [WARNING] 驅動程式可能未正確安裝
)

REM 步驟 3: 建立簡單的 CLI 工具
echo.
echo ========================================
echo 步驟 3: 建立簡單的 CLI 工具
echo ========================================

echo [INFO] 建立簡單的 vddctl.exe...
echo @echo off > "%VDD_DIR%x64\Debug\vddctl.bat"
echo echo VDD CLI Tool (Simple Version) >> "%VDD_DIR%x64\Debug\vddctl.bat"
echo echo ================================ >> "%VDD_DIR%x64\Debug\vddctl.bat"
echo echo. >> "%VDD_DIR%x64\Debug\vddctl.bat"
echo echo Available commands: >> "%VDD_DIR%x64\Debug\vddctl.bat"
echo echo   status    - Check driver status >> "%VDD_DIR%x64\Debug\vddctl.bat"
echo echo   install   - Install driver >> "%VDD_DIR%x64\Debug\vddctl.bat"
echo echo   uninstall - Uninstall driver >> "%VDD_DIR%x64\Debug\vddctl.bat"
echo echo   help      - Show this help >> "%VDD_DIR%x64\Debug\vddctl.bat"
echo echo. >> "%VDD_DIR%x64\Debug\vddctl.bat"
echo if "%%1"=="status" goto :status >> "%VDD_DIR%x64\Debug\vddctl.bat"
echo if "%%1"=="install" goto :install >> "%VDD_DIR%x64\Debug\vddctl.bat"
echo if "%%1"=="uninstall" goto :uninstall >> "%VDD_DIR%x64\Debug\vddctl.bat"
echo if "%%1"=="help" goto :help >> "%VDD_DIR%x64\Debug\vddctl.bat"
echo goto :help >> "%VDD_DIR%x64\Debug\vddctl.bat"
echo. >> "%VDD_DIR%x64\Debug\vddctl.bat"
echo :status >> "%VDD_DIR%x64\Debug\vddctl.bat"
echo echo [INFO] Checking driver status... >> "%VDD_DIR%x64\Debug\vddctl.bat"
echo pnputil /enum-drivers ^| findstr "IddSampleDriver" >> "%VDD_DIR%x64\Debug\vddctl.bat"
echo goto :end >> "%VDD_DIR%x64\Debug\vddctl.bat"
echo. >> "%VDD_DIR%x64\Debug\vddctl.bat"
echo :install >> "%VDD_DIR%x64\Debug\vddctl.bat"
echo echo [INFO] Installing driver... >> "%VDD_DIR%x64\Debug\vddctl.bat"
echo pnputil /add-driver "%DRIVER_DIR%\IddSampleDriver.inf" /install >> "%VDD_DIR%x64\Debug\vddctl.bat"
echo goto :end >> "%VDD_DIR%x64\Debug\vddctl.bat"
echo. >> "%VDD_DIR%x64\Debug\vddctl.bat"
echo :uninstall >> "%VDD_DIR%x64\Debug\vddctl.bat"
echo echo [INFO] Uninstalling driver... >> "%VDD_DIR%x64\Debug\vddctl.bat"
echo pnputil /delete-driver "IddSampleDriver.inf" /uninstall >> "%VDD_DIR%x64\Debug\vddctl.bat"
echo goto :end >> "%VDD_DIR%x64\Debug\vddctl.bat"
echo. >> "%VDD_DIR%x64\Debug\vddctl.bat"
echo :help >> "%VDD_DIR%x64\Debug\vddctl.bat"
echo echo VDD CLI Tool - Simple Version >> "%VDD_DIR%x64\Debug\vddctl.bat"
echo echo Usage: vddctl.bat [command] >> "%VDD_DIR%x64\Debug\vddctl.bat"
echo echo. >> "%VDD_DIR%x64\Debug\vddctl.bat"
echo echo Commands: >> "%VDD_DIR%x64\Debug\vddctl.bat"
echo echo   status    - Check driver status >> "%VDD_DIR%x64\Debug\vddctl.bat"
echo echo   install   - Install driver >> "%VDD_DIR%x64\Debug\vddctl.bat"
echo echo   uninstall - Uninstall driver >> "%VDD_DIR%x64\Debug\vddctl.bat"
echo echo   help      - Show this help >> "%VDD_DIR%x64\Debug\vddctl.bat"
echo echo. >> "%VDD_DIR%x64\Debug\vddctl.bat"
echo echo Examples: >> "%VDD_DIR%x64\Debug\vddctl.bat"
echo echo   vddctl.bat status >> "%VDD_DIR%x64\Debug\vddctl.bat"
echo echo   vddctl.bat install >> "%VDD_DIR%x64\Debug\vddctl.bat"
echo echo   vddctl.bat uninstall >> "%VDD_DIR%x64\Debug\vddctl.bat"
echo goto :end >> "%VDD_DIR%x64\Debug\vddctl.bat"
echo. >> "%VDD_DIR%x64\Debug\vddctl.bat"
echo :end >> "%VDD_DIR%x64\Debug\vddctl.bat"

echo [SUCCESS] 簡單 CLI 工具建立完成

REM 步驟 4: 測試部署
echo.
echo ========================================
echo 步驟 4: 測試部署
echo ========================================

echo [INFO] 測試 CLI 工具...
"%VDD_DIR%x64\Debug\vddctl.bat" status

echo.
echo ========================================
echo 部署完成
echo ========================================
echo.
echo 使用說明:
echo 1. 驅動程式已安裝到系統中
echo 2. 使用 vddctl.bat 管理驅動程式
echo 3. 檢查 Windows 裝置管理員中的虛擬顯示器
echo.
echo 常用命令:
echo   .\x64\Debug\vddctl.bat status     - 檢查狀態
echo   .\x64\Debug\vddctl.bat install    - 重新安裝
echo   .\x64\Debug\vddctl.bat uninstall  - 卸載
echo   .\x64\Debug\vddctl.bat help       - 查看幫助
echo.

pause
