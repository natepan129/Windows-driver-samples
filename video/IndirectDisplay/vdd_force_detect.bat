@echo off
chcp 65001 >nul
echo ========================================
echo VDD 強制偵測工具
echo ========================================

set VDD_DIR=%~dp0
set DRIVER_DIR=%VDD_DIR%x64\Debug\IddSampleDriver

echo [INFO] VDD 目錄: %VDD_DIR%
echo [INFO] 驅動程式目錄: %DRIVER_DIR%

echo.
echo ========================================
echo 步驟 1: 強制重新偵測顯示器
echo ========================================

echo [INFO] 強制重新偵測顯示器...
rundll32.exe user32.dll,UpdatePerUserSystemParameters

echo [INFO] 重新整理顯示器設定...
powershell -Command "Get-WmiObject -Class Win32_DesktopMonitor | ForEach-Object { Write-Host $_.Name }"

echo.
echo ========================================
echo 步驟 2: 重新安裝驅動程式
echo ========================================

echo [INFO] 重新安裝驅動程式...
echo [INFO] 先卸載舊驅動程式...
pnputil /delete-driver "IddSampleDriver.inf" /uninstall

echo [INFO] 等待 3 秒...
timeout /t 3 /nobreak >nul

echo [INFO] 重新安裝驅動程式...
pnputil /add-driver "%DRIVER_DIR%\IddSampleDriver.inf" /install

echo.
echo ========================================
echo 步驟 3: 啟動範例應用程式
echo ========================================

if exist "%VDD_DIR%\x64\Debug\IddSampleApp.exe" (
    echo [INFO] 啟動範例應用程式...
    start "" "%VDD_DIR%\x64\Debug\IddSampleApp.exe"
    
    echo [INFO] 等待 5 秒讓應用程式啟動...
    timeout /t 5 /nobreak >nul
    
    echo [INFO] 檢查虛擬顯示器...
    powershell -Command "Get-WmiObject -Class Win32_DesktopMonitor | Where-Object {$_.Name -like '*Virtual*'} | ForEach-Object { Write-Host 'Found virtual monitor: ' $_.Name }"
) else (
    echo [WARNING] 找不到範例應用程式
)

echo.
echo ========================================
echo 步驟 4: 檢查 Windows 顯示設定
echo ========================================

echo [INFO] 開啟 Windows 顯示設定...
echo [INFO] 請手動檢查 Windows 設定 > 系統 > 顯示
echo [INFO] 查看是否有新的虛擬顯示器

echo.
echo ========================================
echo 步驟 5: 檢查裝置管理員
echo ========================================

echo [INFO] 開啟裝置管理員...
echo [INFO] 請手動檢查裝置管理員 > 顯示器
echo [INFO] 查看是否有 "IddSampleDriver" 相關裝置

echo.
echo ========================================
echo 強制偵測完成
echo ========================================
echo.
echo 如果仍然看不到虛擬顯示器，請嘗試:
echo 1. 重新啟動系統
echo 2. 檢查驅動程式簽名設定
echo 3. 確認 VirtualBox 設定正確
echo 4. 查看 Windows 事件檢視器
echo.
pause
