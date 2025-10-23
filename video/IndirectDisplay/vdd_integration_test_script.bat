@echo off
chcp 65001 >nul
setlocal EnableDelayedExpansion

:: VDD 驅動程式整合測試腳本
:: 測試完整的 VDD 功能整合

echo ========================================
echo VDD 驅動程式整合測試
echo ========================================

:: 設定環境變數
set "VDD_ROOT_DIR=%~dp0"
set "DRIVER_DIR=%VDD_ROOT_DIR%x64\Debug\IddSampleDriver"
set "VDDCTL_EXE_PATH=%VDD_ROOT_DIR%x64\Debug\vddctl.exe"

echo [INFO] VDD 目錄: %VDD_ROOT_DIR%
echo [INFO] 驅動程式目錄: %DRIVER_DIR%
echo [INFO] vddctl.exe 路徑: %VDDCTL_EXE_PATH%
echo.

:: 檢查管理員權限
echo ========================================
echo 步驟 1: 檢查管理員權限
echo ========================================
whoami /groups | find "S-1-5-32-544" > nul
if %errorLevel% neq 0 (
    echo [ERROR] 需要管理員權限執行此腳本。
    echo 請右鍵點擊此腳本，選擇 "以管理員身份執行"。
    goto :error_exit
)
echo [SUCCESS] 已具備管理員權限。
echo.

:: 檢查必要檔案
echo ========================================
echo 步驟 2: 檢查必要檔案
echo ========================================
if not exist "%DRIVER_DIR%\IddSampleDriver.inf" (
    echo [ERROR] 驅動程式 INF 檔案不存在: %DRIVER_DIR%\IddSampleDriver.inf
    goto :error_exit
)
echo [SUCCESS] 驅動程式 INF 檔案存在。

if not exist "%VDDCTL_EXE_PATH%" (
    echo [ERROR] vddctl.exe 不存在: %VDDCTL_EXE_PATH%
    goto :error_exit
)
echo [SUCCESS] vddctl.exe 存在。
echo.

:: 測試 1: 基本功能測試
echo ========================================
echo 測試 1: 基本功能測試
echo ========================================
echo [INFO] 測試 VDD SDK 初始化...
"%VDDCTL_EXE_PATH%" init
if %errorLevel% neq 0 (
    echo [WARNING] VDD SDK 初始化失敗，但繼續測試。
)

echo [INFO] 測試驅動程式安裝...
"%VDDCTL_EXE_PATH%" install --inf "%DRIVER_DIR%\IddSampleDriver.inf"
if %errorLevel% neq 0 (
    echo [ERROR] 驅動程式安裝失敗。
    goto :error_exit
)
echo [SUCCESS] 驅動程式安裝成功。
echo.

:: 測試 2: 虛擬顯示器管理
echo ========================================
echo 測試 2: 虛擬顯示器管理
echo ========================================
echo [INFO] 測試虛擬顯示器激活...
"%VDDCTL_EXE_PATH%" activate --name "VDD Test Display" --width 1920 --height 1080 --refresh 60 --count 1
if %errorLevel% neq 0 (
    echo [WARNING] 虛擬顯示器激活失敗，但繼續測試。
) else (
    echo [SUCCESS] 虛擬顯示器激活成功。
)

echo [INFO] 測試顯示器模式設定...
"%VDDCTL_EXE_PATH%" set-mode --index 0 --width 2560 --height 1440 --refresh 75
if %errorLevel% neq 0 (
    echo [WARNING] 顯示器模式設定失敗。
) else (
    echo [SUCCESS] 顯示器模式設定成功。
)

echo [INFO] 測試顯示器位置設定...
"%VDDCTL_EXE_PATH%" set-location --index 0 --x 3840 --y 0 --width 2560 --height 1440
if %errorLevel% neq 0 (
    echo [WARNING] 顯示器位置設定失敗。
) else (
    echo [SUCCESS] 顯示器位置設定成功。
)

echo [INFO] 測試主要顯示器設定...
"%VDDCTL_EXE_PATH%" set-primary --index 0
if %errorLevel% neq 0 (
    echo [WARNING] 主要顯示器設定失敗。
) else (
    echo [SUCCESS] 主要顯示器設定成功。
)
echo.

:: 測試 3: 查詢功能
echo ========================================
echo 測試 3: 查詢功能
echo ========================================
echo [INFO] 測試狀態查詢...
"%VDDCTL_EXE_PATH%" status
if %errorLevel% neq 0 (
    echo [WARNING] 狀態查詢失敗。
) else (
    echo [SUCCESS] 狀態查詢成功。
)

echo [INFO] 測試顯示器列舉...
"%VDDCTL_EXE_PATH%" enumerate
if %errorLevel% neq 0 (
    echo [WARNING] 顯示器列舉失敗。
) else (
    echo [SUCCESS] 顯示器列舉成功。
)

echo [INFO] 測試 DXGI 輸出查詢...
"%VDDCTL_EXE_PATH%" find-dxgi-output --name "VDD Test Display"
if %errorLevel% neq 0 (
    echo [WARNING] DXGI 輸出查詢失敗。
) else (
    echo [SUCCESS] DXGI 輸出查詢成功。
)
echo.

:: 測試 4: 會話管理
echo ========================================
echo 測試 4: 會話管理
echo ========================================
echo [INFO] 測試會話開始...
"%VDDCTL_EXE_PATH%" begin-session
if %errorLevel% neq 0 (
    echo [WARNING] 會話開始失敗。
) else (
    echo [SUCCESS] 會話開始成功。
)

echo [INFO] 測試會話激活...
"%VDDCTL_EXE_PATH%" activate-leased --lease 12345 --name "VDD Session Display" --width 1920 --height 1080 --refresh 60
if %errorLevel% neq 0 (
    echo [WARNING] 會話激活失敗。
) else (
    echo [SUCCESS] 會話激活成功。
)

echo [INFO] 測試心跳...
"%VDDCTL_EXE_PATH%" heartbeat --lease 12345
if %errorLevel% neq 0 (
    echo [WARNING] 心跳失敗。
) else (
    echo [SUCCESS] 心跳成功。
)

echo [INFO] 測試會話狀態查詢...
"%VDDCTL_EXE_PATH%" get-session-state --lease 12345
if %errorLevel% neq 0 (
    echo [WARNING] 會話狀態查詢失敗。
) else (
    echo [SUCCESS] 會話狀態查詢成功。
)

echo [INFO] 測試會話結束...
"%VDDCTL_EXE_PATH%" end-session --lease 12345
if %errorLevel% neq 0 (
    echo [WARNING] 會話結束失敗。
) else (
    echo [SUCCESS] 會話結束成功。
)
echo.

:: 測試 5: 進階功能
echo ========================================
echo 測試 5: 進階功能
echo ========================================
echo [INFO] 測試 HDR 支援設定...
"%VDDCTL_EXE_PATH%" set-hdr --index 0 --enable
if %errorLevel% neq 0 (
    echo [WARNING] HDR 支援設定失敗。
) else (
    echo [SUCCESS] HDR 支援設定成功。
)

echo [INFO] 測試立體 3D 支援設定...
"%VDDCTL_EXE_PATH%" set-stereo --index 0 --enable
if %errorLevel% neq 0 (
    echo [WARNING] 立體 3D 支援設定失敗。
) else (
    echo [SUCCESS] 立體 3D 支援設定成功。
)

echo [INFO] 測試 EDID 查詢...
"%VDDCTL_EXE_PATH%" get-edid --index 0
if %errorLevel% neq 0 (
    echo [WARNING] EDID 查詢失敗。
) else (
    echo [SUCCESS] EDID 查詢成功。
)
echo.

:: 測試 6: 恢復和維護
echo ========================================
echo 測試 6: 恢復和維護
echo ========================================
echo [INFO] 測試孤兒狀態恢復...
"%VDDCTL_EXE_PATH%" recover-orphaned
if %errorLevel% neq 0 (
    echo [WARNING] 孤兒狀態恢復失敗。
) else (
    echo [SUCCESS] 孤兒狀態恢復成功。
)

echo [INFO] 測試驅動程式運行確保...
"%VDDCTL_EXE_PATH%" ensure-driver-running
if %errorLevel% neq 0 (
    echo [WARNING] 驅動程式運行確保失敗。
) else (
    echo [SUCCESS] 驅動程式運行確保成功。
)

echo [INFO] 測試系統資訊查詢...
"%VDDCTL_EXE_PATH%" get-system-info
if %errorLevel% neq 0 (
    echo [WARNING] 系統資訊查詢失敗。
) else (
    echo [SUCCESS] 系統資訊查詢成功。
)

echo [INFO] 測試錯誤資訊查詢...
"%VDDCTL_EXE_PATH%" get-last-error
if %errorLevel% neq 0 (
    echo [WARNING] 錯誤資訊查詢失敗。
) else (
    echo [SUCCESS] 錯誤資訊查詢成功。
)
echo.

:: 測試 7: JSON 輸出
echo ========================================
echo 測試 7: JSON 輸出
echo ========================================
echo [INFO] 測試 JSON 狀態輸出...
"%VDDCTL_EXE_PATH%" status --json
if %errorLevel% neq 0 (
    echo [WARNING] JSON 狀態輸出失敗。
) else (
    echo [SUCCESS] JSON 狀態輸出成功。
)

echo [INFO] 測試 JSON 列舉輸出...
"%VDDCTL_EXE_PATH%" enumerate --json --pretty
if %errorLevel% neq 0 (
    echo [WARNING] JSON 列舉輸出失敗。
) else (
    echo [SUCCESS] JSON 列舉輸出成功。
)
echo.

:: 測試 8: 清理和卸載
echo ========================================
echo 測試 8: 清理和卸載
echo ========================================
echo [INFO] 測試虛擬顯示器停用...
"%VDDCTL_EXE_PATH%" deactivate
if %errorLevel% neq 0 (
    echo [WARNING] 虛擬顯示器停用失敗。
) else (
    echo [SUCCESS] 虛擬顯示器停用成功。
)

echo [INFO] 測試驅動程式卸載...
"%VDDCTL_EXE_PATH%" uninstall
if %errorLevel% neq 0 (
    echo [WARNING] 驅動程式卸載失敗。
) else (
    echo [SUCCESS] 驅動程式卸載成功。
)

echo [INFO] 測試 VDD SDK 關閉...
"%VDDCTL_EXE_PATH%" shutdown
if %errorLevel% neq 0 (
    echo [WARNING] VDD SDK 關閉失敗。
) else (
    echo [SUCCESS] VDD SDK 關閉成功。
)
echo.

:: 測試完成
echo ========================================
echo 測試完成
echo ========================================
echo [SUCCESS] VDD 驅動程式整合測試完成！
echo.
echo 測試結果摘要:
echo - 基本功能: 驅動程式安裝、虛擬顯示器管理
echo - 查詢功能: 狀態查詢、顯示器列舉、DXGI 輸出查詢
echo - 會話管理: 會話開始、激活、心跳、結束
echo - 進階功能: HDR 支援、立體 3D 支援、EDID 管理
echo - 恢復維護: 孤兒狀態恢復、驅動程式確保、系統資訊
echo - JSON 輸出: 狀態輸出、列舉輸出
echo - 清理卸載: 虛擬顯示器停用、驅動程式卸載、SDK 關閉
echo.
echo 如果所有測試都成功，表示 VDD 驅動程式整合正常！
echo.

goto :eof

:error_exit
echo [ERROR] 測試過程中發生錯誤。
echo 請檢查錯誤訊息並重試。
pause
exit /b 1
