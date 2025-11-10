@echo off
REM ========================================
REM VDD 三個使用場景測試腳本
REM ========================================
REM 測試三個主要使用場景
REM ========================================

setlocal enabledelayedexpansion

set "VDDCTL_PATH=build\bin\Release\vddctl.exe"
set "INF_PATH=x64\Release\IddSampleDriver.inf"

echo ========================================
echo VDD 三個使用場景測試
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

REM 檢查文件
if not exist "%VDDCTL_PATH%" (
    echo [ERROR] vddctl.exe 不存在
    pause
    exit /b 1
)

REM 初始化 SDK
echo [STEP 1] 初始化 SDK...
"%VDDCTL_PATH%" init
if %errorLevel% neq 0 (
    echo [ERROR] SDK 初始化失敗
    pause
    exit /b 1
)

REM 檢查驅動是否已安裝
echo.
echo [STEP 2] 檢查驅動狀態...
"%VDDCTL_PATH%" status | findstr /i "Driver Installed" >nul
if %errorLevel% neq 0 (
    echo [INFO] 驅動未安裝，需要管理員權限安裝...
    net session >nul 2>&1
    if %errorLevel% neq 0 (
        echo [ERROR] 需要管理員權限安裝驅動
        echo 請以管理員身份執行此腳本
        pause
        exit /b 1
    )
    echo [INFO] 安裝驅動...
    "%VDDCTL_PATH%" install --path "%INF_PATH%"
    if %errorLevel% neq 0 (
        echo [ERROR] 驅動安裝失敗
        pause
        exit /b 1
    )
    echo [INFO] 等待 3 秒讓驅動初始化...
    timeout /t 3 /nobreak >nul
)

echo.
echo ========================================
echo 場景 1: VDD 作為顯示代理 (Display Proxy)
echo ========================================
echo [INFO] 激活高刷新率虛擬顯示 (1920x1080@120Hz)...
"%VDDCTL_PATH%" activate --name "VDD Proxy" --width 1920 --height 1080 --refresh 120
if %errorLevel% equ 0 (
    echo [SUCCESS] 場景 1 激活成功
    timeout /t 2 /nobreak >nul
    echo [INFO] 設置顯示位置（擴展模式）...
    "%VDDCTL_PATH%" setlocation --index 0 --x 1920 --y 0 --width 1920 --height 1080
    echo [INFO] 檢查狀態...
    "%VDDCTL_PATH%" status
    echo.
    echo [INFO] 按任意鍵繼續到下一個場景...
    pause >nul
    "%VDDCTL_PATH%" deactivate
    timeout /t 1 /nobreak >nul
) else (
    echo [FAIL] 場景 1 失敗
)

echo.
echo ========================================
echo 場景 2: VDD 作為次要顯示器 (Secondary)
echo ========================================
echo [INFO] 激活高解析度虛擬顯示 (2560x1440@90Hz)...
"%VDDCTL_PATH%" activate --name "VDD Remote" --width 2560 --height 1440 --refresh 90
if %errorLevel% equ 0 (
    echo [SUCCESS] 場景 2 激活成功
    timeout /t 2 /nobreak >nul
    echo [INFO] 設置為擴展模式（右側）...
    "%VDDCTL_PATH%" setlocation --index 0 --x 1920 --y 0 --width 2560 --height 1440
    echo [INFO] 檢查狀態...
    "%VDDCTL_PATH%" status
    echo.
    echo [INFO] 按任意鍵繼續到下一個場景...
    pause >nul
    "%VDDCTL_PATH%" deactivate
    timeout /t 1 /nobreak >nul
) else (
    echo [FAIL] 場景 2 失敗
)

echo.
echo ========================================
echo 場景 3: VDD 作為主顯示器 (Primary)
echo ========================================
echo [WARNING] ⚠️  重要警告 ⚠️
echo [WARNING] 如果您的系統只有一個物理螢幕，將 VDD 設置為主顯示器
echo [WARNING] 會導致物理螢幕變黑，因為主顯示器會切換到虛擬顯示器！
echo [WARNING] 這可能導致您無法看到桌面，需要重啟或使用其他方法恢復。
echo.
echo [INFO] 檢查物理顯示器數量...
"%VDDCTL_PATH%" list > temp_display_list.txt 2>&1
findstr /i /c:"Physical" /c:"Adapter" temp_display_list.txt >nul
set PHYSICAL_COUNT=0
for /f %%i in ('findstr /i /c:"Adapter" temp_display_list.txt ^| find /c /v ""') do set PHYSICAL_COUNT=%%i
del temp_display_list.txt >nul 2>&1

echo [INFO] 檢測到約 %PHYSICAL_COUNT% 個顯示適配器
echo.
echo [QUESTION] 您確定要繼續測試場景 3（設置 VDD 為主顯示器）嗎？
echo [OPTIONS] 
echo   1. 是，繼續測試（僅在有兩個或以上物理螢幕時推薦）
echo   2. 跳過此場景（推薦，如果只有一個物理螢幕）
echo.
set /p SCENARIO3_CHOICE="請選擇 (1/2，默認跳過): "
if "!SCENARIO3_CHOICE!"=="1" (
    echo [INFO] 激活虛擬顯示 (1920x1080@90Hz)...
    "%VDDCTL_PATH%" activate --name "VDD Primary" --width 1920 --height 1080 --refresh 90
    if !errorLevel! equ 0 (
        echo [SUCCESS] 場景 3 激活成功
        timeout /t 2 /nobreak >nul
        echo [WARNING] ⚠️  即將設置 VDD 為主顯示器！
        echo [WARNING] 如果只有一個物理螢幕，這會導致黑畫面！
        echo [INFO] 5 秒後繼續，按 Ctrl+C 取消...
        timeout /t 5 /nobreak >nul
        echo [INFO] 設置為主顯示器...
        "%VDDCTL_PATH%" setprimary --index 0
        echo [INFO] 檢查狀態...
        "%VDDCTL_PATH%" status
        echo.
        echo [WARNING] VDD 現在是主顯示器！
        echo [INFO] 如果出現黑畫面，可以：
        echo   1. 等待 30 秒後自動停用
        echo   2. 使用 Win+P 切換顯示模式
        echo   3. 重啟電腦
        echo.
        echo [INFO] 30 秒後自動停用並完成測試...
        timeout /t 30 /nobreak >nul
        "%VDDCTL_PATH%" deactivate
        timeout /t 1 /nobreak >nul
    ) else (
        echo [FAIL] 場景 3 失敗
    )
) else (
    echo [SKIP] 跳過場景 3（設置主顯示器）
    echo [INFO] 這是安全的選擇，特別是當您只有一個物理螢幕時
)

echo.
echo ========================================
echo 測試完成
echo ========================================
echo [INFO] 關閉 SDK...
"%VDDCTL_PATH%" shutdown

echo.
echo [SUCCESS] 所有場景測試完成！
pause

