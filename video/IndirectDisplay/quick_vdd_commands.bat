@echo off
REM VDD 快速命令腳本
echo ========================================
echo VDD 快速命令
echo ========================================

set VDD_DIR=%~dp0
set CLI_DIR=%VDD_DIR%x64\Debug

echo [INFO] VDD 目錄: %VDD_DIR%
echo [INFO] CLI 目錄: %CLI_DIR%

echo.
echo 可用的快速命令:
echo ========================================
echo 1. 檢查狀態
echo 2. 安裝驅動程式
echo 3. 卸載驅動程式
echo 4. 重新安裝
echo 5. 查看幫助
echo 6. 退出
echo ========================================

:menu
echo.
set /p choice="請選擇操作 (1-6): "

if "%choice%"=="1" goto :status
if "%choice%"=="2" goto :install
if "%choice%"=="3" goto :uninstall
if "%choice%"=="4" goto :reinstall
if "%choice%"=="5" goto :help
if "%choice%"=="6" goto :exit
echo 無效選擇，請重新輸入
goto :menu

:status
echo.
echo ========================================
echo 檢查 VDD 狀態
echo ========================================
echo [INFO] 檢查驅動程式狀態...
pnputil /enum-drivers | findstr "IddSampleDriver"
if %errorLevel% == 0 (
    echo [SUCCESS] 驅動程式已安裝
) else (
    echo [WARNING] 驅動程式未安裝
)
echo.
echo [INFO] 檢查虛擬顯示器...
echo 請檢查 Windows 顯示設定中是否有虛擬顯示器
goto :menu

:install
echo.
echo ========================================
echo 安裝 VDD 驅動程式
echo ========================================
echo [INFO] 安裝驅動程式...
pnputil /add-driver "%VDD_DIR%x64\Debug\IddSampleDriver\IddSampleDriver.inf" /install
if %errorLevel% == 0 (
    echo [SUCCESS] 驅動程式安裝成功
) else (
    echo [ERROR] 驅動程式安裝失敗
)
goto :menu

:uninstall
echo.
echo ========================================
echo 卸載 VDD 驅動程式
echo ========================================
echo [INFO] 卸載驅動程式...
pnputil /delete-driver "IddSampleDriver.inf" /uninstall
if %errorLevel% == 0 (
    echo [SUCCESS] 驅動程式卸載成功
) else (
    echo [WARNING] 驅動程式卸載可能失敗
)
goto :menu

:reinstall
echo.
echo ========================================
echo 重新安裝 VDD 驅動程式
echo ========================================
echo [INFO] 先卸載舊驅動程式...
pnputil /delete-driver "IddSampleDriver.inf" /uninstall
echo [INFO] 安裝新驅動程式...
pnputil /add-driver "%VDD_DIR%x64\Debug\IddSampleDriver\IddSampleDriver.inf" /install
if %errorLevel% == 0 (
    echo [SUCCESS] 驅動程式重新安裝成功
) else (
    echo [ERROR] 驅動程式重新安裝失敗
)
goto :menu

:help
echo.
echo ========================================
echo VDD 幫助
echo ========================================
echo.
echo VDD (Virtual Display Driver) 快速命令
echo ========================================
echo.
echo 功能說明:
echo - 安裝/卸載虛擬顯示器驅動程式
echo - 檢查驅動程式狀態
echo - 管理虛擬顯示器
echo.
echo 使用步驟:
echo 1. 選擇 "安裝驅動程式" 安裝 VDD
echo 2. 檢查 Windows 顯示設定
echo 3. 配置虛擬顯示器
echo.
echo 注意事項:
echo - 需要管理員權限
echo - 可能需要重啟系統
echo - 檢查 Windows 裝置管理員
echo.
echo 故障排除:
echo - 如果安裝失敗，嘗試重新安裝
echo - 檢查驅動程式簽名設定
echo - 查看 Windows 事件檢視器
echo.
goto :menu

:exit
echo.
echo 感謝使用 VDD 快速命令！
echo.
pause
exit /b 0
