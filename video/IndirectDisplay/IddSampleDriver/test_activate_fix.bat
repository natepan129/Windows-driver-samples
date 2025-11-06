@echo off
echo ========================================
echo 測試 Activate 黑屏修復
echo ========================================
echo.

echo [1/3] 檢查當前狀態...
.\build\bin\Release\vddctl.exe status
echo.

echo ========================================
echo 重要提示：
echo   如果黑屏：
echo   - 按 Win+P 切換顯示模式
echo   - 或按 Win+Ctrl+Shift+B 重置顯示
echo ========================================
echo.
pause

echo [2/3] 執行 Activate...
.\build\bin\Release\vddctl.exe activate
echo.

echo [3/3] 等待 3 秒後檢查狀態...
timeout /t 3 /nobreak
echo.

echo ========================================
echo 檢查最終狀態：
echo ========================================
.\build\bin\Release\vddctl.exe status
.\build\bin\Release\vddctl.exe list
echo.

echo ========================================
echo 測試完成！
echo 如果沒有黑屏，則修復成功！
echo ========================================
pause




