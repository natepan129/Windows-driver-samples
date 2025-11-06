@echo off
REM ==============================================================================
REM Phase A+B 完整测试
REM ==============================================================================

echo.
echo ========================================
echo Phase A+B 完整修复测试
echo ========================================
echo.

echo 包含的修复：
echo Phase A (SDK):
echo   - DiUninstallDriverW 旗标修复
echo   - SetupDiGetClassDevsW 旗标修复
echo   - HWID multi-SZ 处理
echo   - Activate 物理显示器检查
echo   - SetPrimary 安全警告
echo.
echo Phase B (Driver):
echo   - ContainerId 稳定性修复
echo.
pause

echo.
echo [步驟 1] 卸载旧驱动
echo ----------------------------------------
.\build\bin\Release\vddctl.exe uninstall
timeout /t 2 /nobreak >nul
echo.

echo [步驟 2] 安装新驱动（包含所有修复）
echo ----------------------------------------
.\build\bin\Release\vddctl.exe install x64\Release\IddSampleDriver\IddSampleDriver.inf
echo.

echo [步驟 3] 激活虚拟显示器
echo ----------------------------------------
echo 观察日志，应该看到：
echo - "Activate: Checking for physical display (safety requirement)..."
echo - "Found physical display"
echo - "Physical display check passed"
echo.
.\build\bin\Release\vddctl.exe activate
echo.

echo [步驟 4] 检查状态
echo ----------------------------------------
.\build\bin\Release\vddctl.exe status
.\build\bin\Release\vddctl.exe list
echo.

echo [步驟 5] 测试鼠标对齐
echo ----------------------------------------
echo 请测试鼠标对齐：
echo 1. 点击桌面不同位置
echo 2. 拖动窗口
echo 3. 点击任务栏图标
echo.
pause

echo.
echo [步驟 6] 记录显示器布局
echo ----------------------------------------
echo 在 Windows 显示设置中：
echo 1. 记录当前虚拟显示器的位置
echo 2. 如果需要，重新排列位置
echo 3. 记住这个布局用于重启后验证
echo.
pause

echo.
echo [步驟 7] 重启测试（ContainerId 验证）
echo ----------------------------------------
echo 系统将重启...
echo.
echo 重启后请：
echo 1. 检查显示器布局是否保持不变
echo 2. 测试鼠标对齐是否仍然准确
echo 3. 不需要重新 activate
echo.
set /p REBOOT="准备好重启了吗？(Y/N): "

if /i "%REBOOT%"=="Y" (
    echo.
    echo 重启中... 5 秒后执行
    shutdown /r /t 5
    echo.
    echo ========================================
    echo 重启后运行：
    echo .\test_after_reboot.bat
    echo ========================================
) else (
    echo.
    echo 跳过重启，稍后手动测试
)

pause


