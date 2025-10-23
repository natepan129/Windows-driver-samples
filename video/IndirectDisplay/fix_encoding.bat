@echo off
chcp 65001 >nul
echo ========================================
echo VDD 狀態檢查 (修復編碼)
echo ========================================

set VDD_DIR=%~dp0
set DRIVER_DIR=%VDD_DIR%x64\Debug\IddSampleDriver

echo [INFO] VDD 目錄: %VDD_DIR%
echo [INFO] 驅動程式目錄: %DRIVER_DIR%

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

echo.
echo ========================================
echo 下一步操作
echo ========================================
echo.
echo 如果驅動程式未安裝，請選擇:
echo 1. 安裝驅動程式
echo 2. 重新安裝驅動程式
echo 3. 退出
echo.

set /p choice="請選擇操作 (1-3): "

if "%choice%"=="1" goto :install
if "%choice%"=="2" goto :reinstall
if "%choice%"=="3" goto :exit
echo 無效選擇，請重新輸入
goto :menu

:install
echo.
echo ========================================
echo 安裝 VDD 驅動程式
echo ========================================
echo [INFO] 安裝驅動程式...
pnputil /add-driver "%DRIVER_DIR%\IddSampleDriver.inf" /install
if %errorLevel% == 0 (
    echo [SUCCESS] 驅動程式安裝成功
) else (
    echo [ERROR] 驅動程式安裝失敗
)
goto :end

:reinstall
echo.
echo ========================================
echo 重新安裝 VDD 驅動程式
echo ========================================
echo [INFO] 先卸載舊驅動程式...
pnputil /delete-driver "IddSampleDriver.inf" /uninstall
echo [INFO] 安裝新驅動程式...
pnputil /add-driver "%DRIVER_DIR%\IddSampleDriver.inf" /install
if %errorLevel% == 0 (
    echo [SUCCESS] 驅動程式重新安裝成功
) else (
    echo [ERROR] 驅動程式重新安裝失敗
)
goto :end

:end
echo.
echo ========================================
echo 操作完成
echo ========================================
echo.
echo 使用說明:
echo 1. 檢查 Windows 顯示設定
echo 2. 查看是否有新的虛擬顯示器
echo 3. 配置虛擬顯示器設定
echo.
pause

:exit
echo.
echo 感謝使用 VDD 工具！
echo.
pause
exit /b 0
