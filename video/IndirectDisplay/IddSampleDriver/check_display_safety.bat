@echo off
REM ========================================
REM VDD 顯示器安全檢查工具
REM ========================================
REM 檢查系統是否適合設置 VDD 為主顯示器
REM ========================================

setlocal

set "VDDCTL_PATH=build\bin\Release\vddctl.exe"

echo ========================================
echo VDD 顯示器安全檢查
echo ========================================
echo.

REM 檢查 vddctl.exe
if not exist "%VDDCTL_PATH%" (
    echo [ERROR] vddctl.exe 不存在
    echo 請先執行 ..\build_both_msbuild.bat
    pause
    exit /b 1
)

REM 初始化 SDK
echo [STEP 1] 初始化 SDK...
"%VDDCTL_PATH%" init >nul 2>&1
if %errorLevel% neq 0 (
    echo [WARNING] SDK 初始化失敗，繼續檢查...
)

echo.
echo [STEP 2] 檢查顯示器配置...
echo.

REM 列出所有顯示器
"%VDDCTL_PATH%" list > temp_display_check.txt 2>&1

REM 檢查是否有物理顯示器
findstr /i /c:"Physical" /c:"Adapter" temp_display_check.txt >nul
if %errorLevel% neq 0 (
    echo [INFO] 無法檢測顯示器信息
    echo [INFO] 顯示完整列表：
    type temp_display_check.txt
) else (
    echo [INFO] 顯示器列表：
    type temp_display_check.txt
)

echo.
echo ========================================
echo 安全建議
echo ========================================
echo.

REM 簡單的物理顯示器檢測（通過檢查非 VDD 適配器）
findstr /i /v /c:"IddSampleDriver" /c:"Virtual" temp_display_check.txt | findstr /i /c:"Adapter" >nul
if %errorLevel% equ 0 (
    echo [SAFE] ✓ 檢測到物理顯示器
    echo [INFO] 系統可能適合設置 VDD 為主顯示器
    echo [INFO] 但仍需確認您有多個物理螢幕
) else (
    echo [WARNING] ⚠️  無法確認物理顯示器數量
    echo [WARNING] 建議：僅在有兩個或以上物理螢幕時設置 VDD 為主顯示器
)

echo.
echo [IMPORTANT] 重要提示：
echo   - 如果只有一個物理螢幕，設置 VDD 為主顯示器會導致黑畫面
echo   - 這是因為主顯示器會切換到虛擬顯示器
echo   - 虛擬顯示器無法在物理螢幕上顯示
echo.
echo [RECOMMENDATION] 推薦做法：
echo   1. 僅在有兩個或以上物理螢幕時使用 setprimary
echo   2. 使用場景 1 和 2（顯示代理和次要顯示器）更安全
echo   3. 如果必須測試場景 3，請確保有備用顯示器或遠程桌面連接
echo.

REM 清理
del temp_display_check.txt >nul 2>&1

REM 關閉 SDK
"%VDDCTL_PATH%" shutdown >nul 2>&1

echo ========================================
echo 檢查完成
echo ========================================
pause

