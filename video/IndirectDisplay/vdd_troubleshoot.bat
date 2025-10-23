@echo off
chcp 65001 >nul
echo ========================================
echo VDD 故障排除工具
echo ========================================

set VDD_DIR=%~dp0
set DRIVER_DIR=%VDD_DIR%x64\Debug\IddSampleDriver

echo [INFO] VDD 目錄: %VDD_DIR%
echo [INFO] 驅動程式目錄: %DRIVER_DIR%

echo.
echo ========================================
echo 步驟 1: 檢查驅動程式安裝狀態
echo ========================================

echo [INFO] 檢查已安裝的驅動程式...
pnputil /enum-drivers | findstr "IddSampleDriver"
if %errorLevel% == 0 (
    echo [SUCCESS] 驅動程式已安裝
) else (
    echo [WARNING] 驅動程式未安裝
)

echo.
echo ========================================
echo 步驟 2: 檢查裝置管理員
echo ========================================

echo [INFO] 檢查裝置管理員中的顯示器...
echo [INFO] 請手動檢查裝置管理員 > 顯示器
echo [INFO] 查看是否有 "IddSampleDriver" 或 "Virtual Display" 相關裝置

echo.
echo ========================================
echo 步驟 3: 強制重新偵測裝置
echo ========================================

echo [INFO] 強制重新偵測顯示器...
rundll32.exe user32.dll,UpdatePerUserSystemParameters

echo [INFO] 重新整理裝置管理員...
echo [INFO] 請在裝置管理員中按 F5 重新整理

echo.
echo ========================================
echo 步驟 4: 檢查 Windows 顯示設定
echo ========================================

echo [INFO] 檢查 Windows 顯示設定...
echo [INFO] 請開啟 Windows 設定 > 系統 > 顯示
echo [INFO] 查看是否有新的虛擬顯示器

echo.
echo ========================================
echo 步驟 5: 重新安裝驅動程式
echo ========================================

echo [INFO] 嘗試重新安裝驅動程式...
echo [INFO] 先卸載舊驅動程式...
pnputil /delete-driver "IddSampleDriver.inf" /uninstall

echo [INFO] 重新安裝驅動程式...
pnputil /add-driver "%DRIVER_DIR%\IddSampleDriver.inf" /install

echo.
echo ========================================
echo 步驟 6: 檢查驅動程式簽名
echo ========================================

echo [INFO] 檢查驅動程式簽名狀態...
bcdedit /enum | findstr "testsigning"
if %errorLevel% == 0 (
    echo [INFO] 測試簽名已啟用
) else (
    echo [WARNING] 測試簽名未啟用
    echo [INFO] 可能需要啟用測試簽名
)

echo.
echo ========================================
echo 步驟 7: 檢查系統事件
echo ========================================

echo [INFO] 檢查系統事件日誌...
echo [INFO] 請檢查 Windows 事件檢視器 > Windows 日誌 > 系統
echo [INFO] 查看是否有與 IddSampleDriver 相關的錯誤

echo.
echo ========================================
echo 步驟 8: 手動啟動範例應用程式
echo ========================================

if exist "%VDD_DIR%\x64\Debug\IddSampleApp.exe" (
    echo [INFO] 找到範例應用程式，嘗試啟動...
    start "" "%VDD_DIR%\x64\Debug\IddSampleApp.exe"
    echo [SUCCESS] 範例應用程式已啟動
    echo [INFO] 請檢查是否出現虛擬顯示器
) else (
    echo [WARNING] 找不到範例應用程式
)

echo.
echo ========================================
echo 故障排除完成
echo ========================================
echo.
echo 如果仍然看不到虛擬顯示器，請嘗試:
echo 1. 重新啟動系統
echo 2. 檢查驅動程式簽名設定
echo 3. 查看 Windows 事件檢視器
echo 4. 確認 VirtualBox 設定正確
echo.
pause
