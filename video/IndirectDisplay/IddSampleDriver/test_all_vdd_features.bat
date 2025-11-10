@echo off
REM ========================================
REM VDD 完整功能測試腳本
REM ========================================
REM 此腳本測試所有 VDD SDK 功能
REM 需要管理員權限執行 install/uninstall 命令
REM ========================================

setlocal enabledelayedexpansion

set "VDDCTL_PATH=build\bin\Release\vddctl.exe"
set "INF_PATH=x64\Release\IddSampleDriver.inf"
set "TEST_PASSED=0"
set "TEST_FAILED=0"
set "TEST_SKIPPED=0"

echo ========================================
echo VDD 完整功能測試
echo ========================================
echo.
echo [IMPORTANT] 重要提示：
echo   如果出現黑畫面，可以使用以下方法恢復：
echo   1. 按 Win+P，然後按右箭頭，按 Enter（切換顯示模式）
echo   2. 按 Ctrl+Alt+Del，按 Tab 3次，按 Enter，按 R（重啟）
echo   3. 詳細恢復方法請查看 BLACK_SCREEN_RECOVERY.md
echo.
pause
echo.

REM 檢查 vddctl.exe 是否存在
if not exist "%VDDCTL_PATH%" (
    echo [ERROR] vddctl.exe 不存在: %VDDCTL_PATH%
    echo 請先執行 ..\build_both_msbuild.bat 建置專案
    pause
    exit /b 1
)

REM 檢查 INF 文件是否存在
if not exist "%INF_PATH%" (
    echo [ERROR] INF 文件不存在: %INF_PATH%
    echo 請先執行 ..\build_both_msbuild.bat 建置專案
    pause
    exit /b 1
)

echo [INFO] 測試工具路徑: %VDDCTL_PATH%
echo [INFO] INF 文件路徑: %INF_PATH%
echo.

REM ========================================
REM 階段 1: 基本命令測試
REM ========================================
echo ========================================
echo 階段 1: 基本命令測試
echo ========================================
echo.

REM 1.1 測試 version 命令
echo [TEST 1.1] 測試 version 命令...
"%VDDCTL_PATH%" version
if %errorLevel% equ 0 (
    echo [PASS] version 命令執行成功
    set /a TEST_PASSED+=1
) else (
    echo [FAIL] version 命令執行失敗
    set /a TEST_FAILED+=1
)
echo.

REM 1.2 測試 help 命令
echo [TEST 1.2] 測試 help 命令...
"%VDDCTL_PATH%" help >nul 2>&1
if %errorLevel% equ 0 (
    echo [PASS] help 命令執行成功
    set /a TEST_PASSED+=1
) else (
    echo [FAIL] help 命令執行失敗
    set /a TEST_FAILED+=1
)
echo.

REM ========================================
REM 階段 2: SDK 管理測試
REM ========================================
echo ========================================
echo 階段 2: SDK 管理測試
echo ========================================
echo.

REM 2.1 測試 init 命令
echo [TEST 2.1] 測試 init 命令...
"%VDDCTL_PATH%" init
if %errorLevel% equ 0 (
    echo [PASS] init 命令執行成功
    set /a TEST_PASSED+=1
) else (
    echo [FAIL] init 命令執行失敗
    set /a TEST_FAILED+=1
)
echo.

REM 2.2 測試 status 命令
echo [TEST 2.2] 測試 status 命令...
"%VDDCTL_PATH%" status
if %errorLevel% equ 0 (
    echo [PASS] status 命令執行成功
    set /a TEST_PASSED+=1
) else (
    echo [FAIL] status 命令執行失敗
    set /a TEST_FAILED+=1
)
echo.

REM ========================================
REM 階段 3: 驅動管理測試
REM ========================================
echo ========================================
echo 階段 3: 驅動管理測試
echo ========================================
echo [WARNING] 此階段需要管理員權限
echo.

REM 檢查是否為管理員
net session >nul 2>&1
if %errorLevel% neq 0 (
    echo [SKIP] 非管理員權限，跳過驅動安裝/卸載測試
    echo [INFO] 請以管理員身份執行此腳本以測試完整功能
    set /a TEST_SKIPPED+=2
    goto :test_display_management
)

REM 3.1 檢查驅動是否已安裝
echo [TEST 3.1] 檢查驅動安裝狀態...
"%VDDCTL_PATH%" status | findstr /i "Driver Installed" >nul
if %errorLevel% equ 0 (
    echo [INFO] 驅動已安裝，跳過安裝測試
    set /a TEST_SKIPPED+=1
) else (
    echo [TEST 3.1] 測試 install 命令...
    "%VDDCTL_PATH%" install --path "%INF_PATH%"
    if %errorLevel% equ 0 (
        echo [PASS] install 命令執行成功
        set /a TEST_PASSED+=1
        echo [INFO] 等待 3 秒讓驅動初始化...
        timeout /t 3 /nobreak >nul
    ) else (
        echo [FAIL] install 命令執行失敗
        set /a TEST_FAILED+=1
    )
)
echo.

REM 3.2 再次檢查狀態
echo [TEST 3.2] 檢查安裝後的狀態...
"%VDDCTL_PATH%" status
if %errorLevel% equ 0 (
    echo [PASS] status 命令執行成功
    set /a TEST_PASSED+=1
) else (
    echo [FAIL] status 命令執行失敗
    set /a TEST_FAILED+=1
)
echo.

REM ========================================
REM 階段 4: 虛擬顯示管理測試
REM ========================================
:test_display_management
echo ========================================
echo 階段 4: 虛擬顯示管理測試
echo ========================================
echo.

REM 4.1 測試 activate 命令
echo [TEST 4.1] 測試 activate 命令 (1920x1080@60Hz)...
"%VDDCTL_PATH%" activate --name "VDD Test" --width 1920 --height 1080 --refresh 60
if %errorLevel% equ 0 (
    echo [PASS] activate 命令執行成功
    set /a TEST_PASSED+=1
    echo [INFO] 等待 2 秒讓顯示器初始化...
    timeout /t 2 /nobreak >nul
) else (
    echo [FAIL] activate 命令執行失敗
    set /a TEST_FAILED+=1
)
echo.

REM 4.2 測試 list 命令
echo [TEST 4.2] 測試 list 命令...
"%VDDCTL_PATH%" list
if %errorLevel% equ 0 (
    echo [PASS] list 命令執行成功
    set /a TEST_PASSED+=1
) else (
    echo [FAIL] list 命令執行失敗
    set /a TEST_FAILED+=1
)
echo.

REM 4.3 測試 status 命令（激活後）
echo [TEST 4.3] 測試 status 命令（激活後）...
"%VDDCTL_PATH%" status
if %errorLevel% equ 0 (
    echo [PASS] status 命令執行成功
    set /a TEST_PASSED+=1
) else (
    echo [FAIL] status 命令執行失敗
    set /a TEST_FAILED+=1
)
echo.

REM ========================================
REM 階段 5: 顯示配置測試
REM ========================================
echo ========================================
echo 階段 5: 顯示配置測試
echo ========================================
echo.

REM 5.1 測試 setmode 命令
echo [TEST 5.1] 測試 setmode 命令 (2560x1440@75Hz)...
"%VDDCTL_PATH%" setmode --index 0 --width 2560 --height 1440 --refresh 75
if %errorLevel% equ 0 (
    echo [PASS] setmode 命令執行成功
    set /a TEST_PASSED+=1
    echo [INFO] 等待 1 秒讓模式切換...
    timeout /t 1 /nobreak >nul
) else (
    echo [FAIL] setmode 命令執行失敗
    set /a TEST_FAILED+=1
)
echo.

REM 5.2 測試 setlocation 命令
echo [TEST 5.2] 測試 setlocation 命令...
"%VDDCTL_PATH%" setlocation --index 0 --x 1920 --y 0 --width 2560 --height 1440
if %errorLevel% equ 0 (
    echo [PASS] setlocation 命令執行成功
    set /a TEST_PASSED+=1
    echo [INFO] 等待 1 秒讓位置更新...
    timeout /t 1 /nobreak >nul
) else (
    echo [FAIL] setlocation 命令執行失敗
    set /a TEST_FAILED+=1
)
echo.

REM 5.3 測試 setprimary 命令（帶安全警告）
echo [TEST 5.3] 測試 setprimary 命令...
echo [WARNING] ⚠️  重要警告 ⚠️
echo [WARNING] 如果您的系統只有一個物理螢幕，將 VDD 設置為主顯示器
echo [WARNING] 會導致物理螢幕變黑，因為主顯示器會切換到虛擬顯示器！
echo [WARNING] 這可能導致您無法看到桌面。
echo.
echo [QUESTION] 您確定要測試 setprimary 命令嗎？
echo [OPTIONS] 
echo   1. 是，繼續測試（僅在有兩個或以上物理螢幕時推薦）
echo   2. 跳過此測試（推薦，如果只有一個物理螢幕）
echo.
set /p SETPRIMARY_CHOICE="請選擇 (1/2，默認跳過): "
if "!SETPRIMARY_CHOICE!"=="1" (
    echo [INFO] 執行 setprimary 命令...
    "%VDDCTL_PATH%" setprimary --index 0
    if !errorLevel! equ 0 (
        echo [PASS] setprimary 命令執行成功
        set /a TEST_PASSED+=1
        echo [INFO] 等待 1 秒讓主顯示器切換...
        timeout /t 1 /nobreak >nul
        echo [WARNING] 如果出現黑畫面，請使用 Win+P 切換顯示模式或重啟電腦
    ) else (
        echo [FAIL] setprimary 命令執行失敗
        set /a TEST_FAILED+=1
    )
) else (
    echo [SKIP] 跳過 setprimary 測試（安全選擇）
    set /a TEST_SKIPPED+=1
)
echo.

REM 5.4 測試多種解析度切換
echo [TEST 5.4] 測試多種解析度切換...
echo [INFO] 切換到 1920x1080@60Hz...
"%VDDCTL_PATH%" setmode --index 0 --width 1920 --height 1080 --refresh 60
timeout /t 1 /nobreak >nul
echo [INFO] 切換到 3840x2160@60Hz (4K)...
"%VDDCTL_PATH%" setmode --index 0 --width 3840 --height 2160 --refresh 60
timeout /t 1 /nobreak >nul
echo [INFO] 切換回 1920x1080@60Hz...
"%VDDCTL_PATH%" setmode --index 0 --width 1920 --height 1080 --refresh 60
if %errorLevel% equ 0 (
    echo [PASS] 多種解析度切換測試成功
    set /a TEST_PASSED+=1
) else (
    echo [FAIL] 多種解析度切換測試失敗
    set /a TEST_FAILED+=1
)
echo.

REM ========================================
REM 階段 6: 清理測試
REM ========================================
echo ========================================
echo 階段 6: 清理測試
echo ========================================
echo.

REM 6.1 測試 deactivate 命令
echo [TEST 6.1] 測試 deactivate 命令...
"%VDDCTL_PATH%" deactivate
if %errorLevel% equ 0 (
    echo [PASS] deactivate 命令執行成功
    set /a TEST_PASSED+=1
    echo [INFO] 等待 1 秒讓顯示器停用...
    timeout /t 1 /nobreak >nul
) else (
    echo [FAIL] deactivate 命令執行失敗
    set /a TEST_FAILED+=1
)
echo.

REM 6.2 測試 shutdown 命令
echo [TEST 6.2] 測試 shutdown 命令...
"%VDDCTL_PATH%" shutdown
if %errorLevel% equ 0 (
    echo [PASS] shutdown 命令執行成功
    set /a TEST_PASSED+=1
) else (
    echo [FAIL] shutdown 命令執行失敗
    set /a TEST_FAILED+=1
)
echo.

REM 6.3 可選：測試 uninstall 命令（僅在管理員模式下）
net session >nul 2>&1
if %errorLevel% equ 0 (
    echo [TEST 6.3] 測試 uninstall 命令（可選）...
    echo [WARNING] 此操作將卸載驅動程式
    echo [INFO] 按任意鍵繼續卸載，或按 Ctrl+C 取消...
    pause >nul
    "%VDDCTL_PATH%" uninstall
    if %errorLevel% equ 0 (
        echo [PASS] uninstall 命令執行成功
        set /a TEST_PASSED+=1
    ) else (
        echo [FAIL] uninstall 命令執行失敗
        set /a TEST_FAILED+=1
    )
    echo.
)

REM ========================================
REM 測試結果總結
REM ========================================
echo ========================================
echo 測試結果總結
echo ========================================
echo [PASS] 通過: %TEST_PASSED%
echo [FAIL] 失敗: %TEST_FAILED%
echo [SKIP] 跳過: %TEST_SKIPPED%
echo.

set /a TOTAL_TESTS=%TEST_PASSED%+%TEST_FAILED%+%TEST_SKIPPED%
if %TOTAL_TESTS% equ 0 (
    echo [ERROR] 沒有執行任何測試
    pause
    exit /b 1
)

set /a PASS_RATE=(%TEST_PASSED% * 100) / %TOTAL_TESTS%
echo [INFO] 通過率: %PASS_RATE%%%
echo.

if %TEST_FAILED% equ 0 (
    echo [SUCCESS] 所有測試通過！
    exit /b 0
) else (
    echo [WARNING] 有 %TEST_FAILED% 個測試失敗
    exit /b 1
)

