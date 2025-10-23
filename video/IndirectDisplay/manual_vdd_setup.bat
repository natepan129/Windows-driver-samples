@echo off
chcp 65001 >nul
echo ========================================
echo VDD 手動設定工具
echo ========================================

set VDD_DIR=%~dp0
set DRIVER_DIR=%VDD_DIR%x64\Debug\IddSampleDriver

echo [INFO] VDD 目錄: %VDD_DIR%
echo [INFO] 驅動程式目錄: %DRIVER_DIR%

echo.
echo ========================================
echo 手動安裝 VDD 驅動程式
echo ========================================

echo [INFO] 手動執行安裝命令...
echo.
echo 執行命令: pnputil /add-driver "%DRIVER_DIR%\IddSampleDriver.inf" /install
echo.

pnputil /add-driver "%DRIVER_DIR%\IddSampleDriver.inf" /install

echo.
echo ========================================
echo 檢查安裝結果
echo ========================================

echo [INFO] 檢查驅動程式安裝狀態...
pnputil /enum-drivers | findstr "IddSampleDriver"

if %errorLevel% == 0 (
    echo.
    echo [SUCCESS] 驅動程式安裝成功！
    echo.
    echo 下一步操作:
    echo 1. 開啟 Windows 設定 > 系統 > 顯示
    echo 2. 查看是否有新的虛擬顯示器
    echo 3. 配置虛擬顯示器設定
    echo 4. 測試虛擬顯示器功能
) else (
    echo.
    echo [WARNING] 驅動程式可能未正確安裝
    echo.
    echo 故障排除:
    echo 1. 檢查是否以管理員身份執行
    echo 2. 檢查驅動程式檔案是否存在
    echo 3. 嘗試重新安裝
    echo 4. 查看 Windows 事件檢視器
)

echo.
echo ========================================
echo 手動設定完成
echo ========================================
echo.
echo 如果安裝成功，您應該能在 Windows 顯示設定中看到新的虛擬顯示器
echo.
pause
