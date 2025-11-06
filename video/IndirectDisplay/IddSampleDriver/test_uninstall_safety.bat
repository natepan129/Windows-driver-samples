@echo off
REM ==============================================================================
REM 测试 UninstallDriver 黑屏防护（一致性修复验证）
REM ==============================================================================

echo.
echo ========================================
echo UninstallDriver 安全测试
echo 目标：验证 Hardware ID 匹配一致性
echo ========================================
echo.

echo [警告] 此测试将卸载虚拟显示器驱动
echo 请确保：
echo 1. 物理显示器已连接并激活
echo 2. 不是在虚拟显示器上运行此脚本
echo 3. 已保存所有工作
echo.
pause

echo.
echo [步驟 1] 检查当前状态
echo ----------------------------------------
.\build\bin\Release\vddctl.exe status
echo.

echo [步驟 2] 列出显示器
echo ----------------------------------------
.\build\bin\Release\vddctl.exe list
echo.

echo [步驟 3] 卸载驱动（带黑屏防护）
echo ----------------------------------------
echo 注意观察日志，应该看到：
echo - "Found IddSampleDriver device via Hardware ID"
echo - "Skipping virtual display: \\.\DISPLAYX"
echo - "Found physical display: \\.\DISPLAY1"
echo - "Physical display set as primary successfully"
echo.

.\build\bin\Release\vddctl.exe uninstall

echo.
echo ========================================
echo 测试完成
echo ========================================
echo.
set /p SUCCESS="屏幕是否正常（没有黑屏）？(Y/N): "

if /i "%SUCCESS%"=="Y" (
    echo.
    echo ✅ 测试通过！UninstallDriver 黑屏防护正常工作
    echo.
    echo 如果需要重新安装驱动：
    echo .\build\bin\Release\vddctl.exe install x64\Release\IddSampleDriver\IddSampleDriver.inf
    echo .\build\bin\Release\vddctl.exe activate
    echo.
) else (
    echo.
    echo ❌ 测试失败 - 出现黑屏或其他问题
    echo.
    echo 紧急恢复步骤：
    echo 1. 按 Win+P 切换显示模式
    echo 2. 重启系统
    echo 3. 进入安全模式卸载驱动
    echo.
)

pause

