@echo off
REM ==============================================================================
REM 测试坐标持久化（重启后不重新激活）
REM ==============================================================================

echo.
echo ========================================
echo 坐标持久化测试
echo 目标：验证 CDS_UPDATEREGISTRY 是否有效
echo ========================================
echo.

echo [步驟 1] 检查驱动状态
echo ----------------------------------------
.\build\bin\Release\vddctl.exe status
echo.

echo [步驟 2] 列出显示器
echo ----------------------------------------
.\build\bin\Release\vddctl.exe list
echo.

echo ========================================
echo 关键测试：不要重新激活
echo ========================================
echo.
echo 请直接测试鼠标对齐：
echo 1. 点击桌面不同位置（左上、右下、中间）
echo 2. 拖动窗口
echo 3. 点击任务栏图标
echo.
echo ========================================
echo 重要：不要运行 vddctl activate！
echo ========================================
echo.

pause

echo.
echo ========================================
echo 请回答以下问题：
echo ========================================
echo.
set /p ALIGNED="鼠标是否准确对齐？(Y/N): "

if /i "%ALIGNED%"=="Y" (
    echo.
    echo ========================================
    echo ✅ 测试通过！
    echo ========================================
    echo.
    echo CDS_UPDATEREGISTRY 成功持久化了坐标！
    echo 这意味着：
    echo - 不需要开机自启动
    echo - 不需要每次重启后手动激活
    echo - 用户可以按需使用 activate/deactivate
    echo.
    echo 🎉 问题完全解决！
    echo.
) else (
    echo.
    echo ========================================
    echo ❌ 测试失败
    echo ========================================
    echo.
    echo CDS_UPDATEREGISTRY 没有持久化坐标
    echo.
    echo 需要实现更强的持久化机制：
    echo - 方案 A：直接写注册表（HKLM\System\CurrentControlSet\Control\GraphicsDrivers\Configuration）
    echo - 方案 B：Driver.cpp 实现稳定的 ContainerId
    echo.
    echo 请运行 vddctl activate 来临时修复对齐问题
    echo.
)

pause


