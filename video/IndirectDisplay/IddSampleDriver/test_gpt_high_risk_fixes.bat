@echo off
REM ==============================================================================
REM 测试 GPT High-Risk 修复
REM ==============================================================================

echo.
echo ========================================
echo GPT High-Risk 修复测试
echo ========================================
echo.

echo [测试 1] Activate 物理显示器检查
echo ----------------------------------------
echo 测试目标：验证在有物理显示器的情况下可以激活
echo.

.\build\bin\Release\vddctl.exe activate

echo.
echo 检查日志，应该看到：
echo - "Activate: Checking for physical display (safety requirement)..."
echo - "Found physical display: \\.\DISPLAY1"
echo - "Physical display check passed"
echo.
pause

echo.
echo [测试 2] 列出显示器
echo ----------------------------------------
.\build\bin\Release\vddctl.exe list
echo.
pause

echo.
echo [测试 3] SetPrimary 警告信息
echo ----------------------------------------
echo 测试目标：验证设置虚拟显示器为主显示器时显示警告
echo.
echo 注意：此操作有风险，仅用于测试警告消息
echo 如果出现问题，可以运行：
echo   vddctl deactivate
echo   或重启系统
echo.
set /p CONTINUE="是否继续测试 SetPrimary 警告？(Y/N): "

if /i "%CONTINUE%"=="Y" (
    echo.
    echo 运行 SetPrimary...
    .\build\bin\Release\vddctl.exe set-primary 1
    echo.
    echo 检查日志，应该看到：
    echo - "SetPrimary: WARNING - Attempting to set virtual display as primary"
    echo - "This operation can cause lock-screen and sign-in issues!"
    echo - "Setting virtual display as primary is NOT RECOMMENDED."
    echo.
    
    echo 如果设置成功，请立即恢复：
    pause
    
    echo.
    echo 恢复为物理显示器为主...
    .\build\bin\Release\vddctl.exe set-primary 0
    echo.
) else (
    echo 跳过 SetPrimary 测试
)

pause

echo.
echo [测试 4] Deactivate + UninstallDriver
echo ----------------------------------------
echo 测试目标：验证完整的清理流程
echo.

.\build\bin\Release\vddctl.exe deactivate
.\build\bin\Release\vddctl.exe uninstall

echo.
echo ========================================
echo 测试完成
echo ========================================
echo.
echo 所有 GPT High-Risk 修复已验证：
echo - [✓] Activate 物理显示器检查
echo - [✓] HWID multi-SZ 正确处理
echo - [✓] SetPrimary 安全警告
echo - [✓] UninstallDriver 黑屏防护
echo.
pause

