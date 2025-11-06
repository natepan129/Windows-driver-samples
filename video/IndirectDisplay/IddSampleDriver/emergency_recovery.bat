@echo off
chcp 65001 > nul
REM ========================================
REM 緊急恢復腳本 - 如果出現黑屏使用此腳本
REM ========================================
REM 
REM 使用方法（黑屏時）：
REM 1. 按 Win+P，選擇"僅電腦屏幕"或"複製"
REM 2. 或者通過遠程桌面/SSH 執行此腳本
REM 3. 或者重啟到安全模式執行此腳本
REM
REM ========================================

echo.
echo ========================================
echo 緊急恢復工具
echo ========================================
echo.
echo 此腳本將：
echo 1. 停用虛擬顯示驅動
echo 2. 重置顯示配置到 Windows 默認
echo 3. 恢復主顯示器為唯一顯示
echo.
echo 按任意鍵開始恢復，或 Ctrl+C 取消
pause > nul

echo.
echo [步驟 1] 停用虛擬顯示驅動...
echo ----------------------------------------
.\build\bin\Release\vddctl.exe deactivate
if %ERRORLEVEL% EQU 0 (
    echo ✓ 驅動已停用
) else (
    echo ⚠ 驅動停用失敗，繼續嘗試其他恢復步驟...
)

echo.
echo [步驟 2] 重置顯示配置到單一顯示器...
echo ----------------------------------------
REM 使用 DisplaySwitch.exe 強制切換到僅主顯示器
DisplaySwitch.exe /internal
timeout /t 2 > nul
echo ✓ 已切換到內部顯示器模式

echo.
echo [步驟 3] 驗證主顯示器狀態...
echo ----------------------------------------
.\build\bin\Release\vddctl.exe status
.\build\bin\Release\vddctl.exe list

echo.
echo ========================================
echo 恢復完成
echo ========================================
echo.
echo 如果屏幕仍然黑屏：
echo.
echo 選項 A - 使用 Windows 內建工具：
echo   1. 按 Win+P 多次循環顯示模式
echo   2. 等待 15 秒讓 Windows 自動恢復
echo.
echo 選項 B - 完全移除驅動：
echo   執行: .\build\bin\Release\vddctl.exe uninstall
echo.
echo 選項 C - 重啟到安全模式：
echo   1. 重啟電腦
echo   2. 按 F8 進入安全模式
echo   3. 在安全模式中卸載驅動
echo.
echo 選項 D - 使用系統還原（如果有創建還原點）：
echo   1. 重啟電腦
echo   2. 按 F8 選擇"修復您的電腦"
echo   3. 選擇系統還原
echo.
pause


