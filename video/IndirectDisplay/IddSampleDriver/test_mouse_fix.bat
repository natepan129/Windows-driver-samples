@echo off
REM ==============================================================================
REM 测试鼠标偏移修复（Task 1.3 + 1.1）
REM ==============================================================================

echo.
echo ========================================
echo 测试 1.3 + 1.1: 鼠标坐标对齐修复
echo ========================================
echo.

echo [步驟 1] 当前系统状态
echo ----------------------------------------
.\build\bin\Release\vddctl.exe status
echo.

echo [步驟 2] 列出所有显示器（应使用 Hardware ID 匹配）
echo ----------------------------------------
.\build\bin\Release\vddctl.exe list
echo.

pause

echo.
echo ========================================
echo [步驟 3] 测试 Activate（坐标不重叠）
echo ========================================
echo ⚠️ 关键测试：
echo   - 虚拟显示器应该在 EXTEND 模式
echo   - 坐标应该不重叠 (例如 1920, 0)
echo   - 鼠标点击位置应该正确
echo ========================================
echo.

.\build\bin\Release\vddctl.exe activate

echo.
echo 等待 3 秒让拓扑稳定...
timeout /t 3 /nobreak >nul

echo.
echo [步驟 4] 检查激活后的状态
echo ----------------------------------------
.\build\bin\Release\vddctl.exe status
.\build\bin\Release\vddctl.exe list
echo.

echo ========================================
echo 测试指示：
echo 1. 请点击屏幕上的不同位置
echo 2. 确认鼠标光标和点击位置对齐
echo 3. 如果鼠标位置正确，修复成功！✓
echo ========================================
pause

echo.
echo ========================================
echo [步驟 5] 测试 Uninstall（防止黑屏）
echo ========================================
echo ⚠️ 关键测试：
echo   - 物理显示器应该先设为主显示器
echo   - 移除虚拟显示器时不应该黑屏
echo ========================================
echo.

pause
echo 执行 Uninstall...
.\build\bin\Release\vddctl.exe uninstall x64\Release\IddSampleDriver\IddSampleDriver.inf

echo.
echo 等待 2 秒...
timeout /t 2 /nobreak >nul

echo.
echo [步驟 6] 检查 Uninstall 后的状态
echo ----------------------------------------
.\build\bin\Release\vddctl.exe status
echo.

echo ========================================
echo 测试结果总结：
echo ----------------------------------------
echo 1. ✓ 鼠标点击位置是否正确？
echo 2. ✓ Uninstall 时是否没有黑屏？
echo 3. ✓ 物理显示器是否仍然正常显示？
echo ========================================
echo.
echo 如果以上三点都正常，修复成功！✅
echo ========================================
pause



