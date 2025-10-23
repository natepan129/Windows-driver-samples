@echo off
chcp 65001 >nul
echo ========================================
echo VDD 直接安裝工具 (不需要 MSBuild)
echo ========================================

set VDD_DIR=%~dp0
set DRIVER_DIR=%VDD_DIR%x64\Debug\IddSampleDriver

echo [INFO] VDD 目錄: %VDD_DIR%
echo [INFO] 驅動程式目錄: %DRIVER_DIR%

echo.
echo ========================================
echo 步驟 1: 檢查管理員權限
echo ========================================

net session >nul 2>&1
if %errorLevel% == 0 (
    echo [SUCCESS] 管理員權限確認
) else (
    echo [ERROR] 需要管理員權限
    echo [INFO] 請以管理員身份執行此腳本
    pause
    exit /b 1
)

echo.
echo ========================================
echo 步驟 2: 檢查驅動程式檔案
echo ========================================

if exist "%DRIVER_DIR%\IddSampleDriver.dll" (
    echo [SUCCESS] 找到驅動程式 DLL: IddSampleDriver.dll
) else (
    echo [ERROR] 找不到驅動程式 DLL
    echo [INFO] 請先建置專案或檢查檔案路徑
    pause
    exit /b 1
)

if exist "%DRIVER_DIR%\IddSampleDriver.inf" (
    echo [SUCCESS] 找到驅動程式 INF: IddSampleDriver.inf
) else (
    echo [ERROR] 找不到驅動程式 INF
    echo [INFO] 請先建置專案或檢查檔案路徑
    pause
    exit /b 1
)

echo.
echo ========================================
echo 步驟 3: 安裝驅動程式
echo ========================================

echo [INFO] 開始安裝 VDD 驅動程式...
echo [INFO] 使用 pnputil 安裝驅動程式...

pnputil /add-driver "%DRIVER_DIR%\IddSampleDriver.inf" /install

if %errorLevel% == 0 (
    echo [SUCCESS] 驅動程式安裝成功！
) else (
    echo [WARNING] 驅動程式安裝可能失敗
    echo [INFO] 錯誤代碼: %errorLevel%
)

echo.
echo ========================================
echo 步驟 4: 驗證安裝
echo ========================================

echo [INFO] 檢查驅動程式是否已安裝...
pnputil /enum-drivers | findstr "IddSampleDriver"
if %errorLevel% == 0 (
    echo [SUCCESS] 驅動程式已成功安裝
) else (
    echo [WARNING] 驅動程式可能未正確安裝
)

echo.
echo ========================================
echo 步驟 5: 後續操作
echo ========================================

echo.
echo 安裝完成！請執行以下操作:
echo.
echo 1. 檢查 Windows 顯示設定:
echo    - 開啟 Windows 設定
echo    - 系統 > 顯示
echo    - 查看是否有新的虛擬顯示器
echo.
echo 2. 如果沒有看到虛擬顯示器:
echo    - 嘗試重新啟動系統
echo    - 檢查 Windows 裝置管理員
echo    - 查看 Windows 事件檢視器
echo.
echo 3. 測試虛擬顯示器:
echo    - 執行 .\x64\Debug\IddSampleApp.exe
echo    - 檢查虛擬顯示器是否正常工作
echo.

echo ========================================
echo 安裝完成
echo ========================================
echo.
echo 感謝使用 VDD 直接安裝工具！
echo.
pause
