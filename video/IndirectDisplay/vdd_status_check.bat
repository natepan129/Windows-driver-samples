@echo off
chcp 65001 >nul
echo ========================================
echo VDD 狀態檢查工具
echo ========================================

set VDD_DIR=%~dp0
set DRIVER_DIR=%VDD_DIR%x64\Debug\IddSampleDriver

echo [INFO] VDD 目錄: %VDD_DIR%
echo [INFO] 驅動程式目錄: %DRIVER_DIR%

echo.
echo ========================================
echo 步驟 1: 檢查驅動程式檔案
echo ========================================

if exist "%DRIVER_DIR%\IddSampleDriver.dll" (
    echo [SUCCESS] 找到驅動程式 DLL: IddSampleDriver.dll
) else (
    echo [ERROR] 找不到驅動程式 DLL
    goto :error
)

if exist "%DRIVER_DIR%\IddSampleDriver.inf" (
    echo [SUCCESS] 找到驅動程式 INF: IddSampleDriver.inf
) else (
    echo [ERROR] 找不到驅動程式 INF
    goto :error
)

if exist "%DRIVER_DIR%\iddsampledriver.cat" (
    echo [SUCCESS] 找到驅動程式 CAT: iddsampledriver.cat
) else (
    echo [WARNING] 找不到驅動程式 CAT (可能正常)
)

echo.
echo ========================================
echo 步驟 2: 檢查驅動程式安裝狀態
echo ========================================

echo [INFO] 檢查已安裝的驅動程式...
pnputil /enum-drivers | findstr "IddSampleDriver"
if %errorLevel% == 0 (
    echo [SUCCESS] 驅動程式已安裝
) else (
    echo [WARNING] 驅動程式未安裝
    echo [INFO] 需要安裝驅動程式
)

echo.
echo ========================================
echo 步驟 3: 檢查系統權限
echo ========================================

net session >nul 2>&1
if %errorLevel% == 0 (
    echo [SUCCESS] 管理員權限確認
) else (
    echo [ERROR] 需要管理員權限
    echo [INFO] 請以管理員身份執行此腳本
    goto :error
)

echo.
echo ========================================
echo 步驟 4: 建議操作
echo ========================================

echo.
echo 根據檢查結果，建議執行以下操作:
echo.
echo 1. 如果驅動程式未安裝:
echo    pnputil /add-driver "%DRIVER_DIR%\IddSampleDriver.inf" /install
echo.
echo 2. 如果驅動程式已安裝但需要重新安裝:
echo    pnputil /delete-driver "IddSampleDriver.inf" /uninstall
echo    pnputil /add-driver "%DRIVER_DIR%\IddSampleDriver.inf" /install
echo.
echo 3. 檢查 Windows 顯示設定:
echo    - 開啟 Windows 設定
echo    - 系統 > 顯示
echo    - 查看是否有新的虛擬顯示器
echo.

echo ========================================
echo 狀態檢查完成
echo ========================================
echo.
echo 下一步: 執行 fix_encoding.bat 進行安裝
echo.
pause
goto :end

:error
echo.
echo ========================================
echo 錯誤處理
echo ========================================
echo.
echo 發現問題，請檢查:
echo 1. 是否以管理員身份執行
echo 2. 驅動程式檔案是否存在
echo 3. 系統權限是否足夠
echo.
pause
exit /b 1

:end
echo.
echo 感謝使用 VDD 狀態檢查工具！
echo.
pause
exit /b 0
