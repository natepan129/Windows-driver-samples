@echo off
chcp 65001 >nul
echo ========================================
echo 清理旧设备（需要管理员权限）
echo ========================================
echo.

REM 检查管理员权限
net session >nul 2>&1
if %errorLevel% neq 0 (
    echo 错误：需要管理员权限！
    echo 请右键此脚本，选择"以管理员身份运行"
    pause
    exit /b 1
)

echo 正在删除旧设备...
echo.

REM 使用 pnputil 删除所有 IddSampleDriver 设备
pnputil /remove-device "ROOT\IDDSAMPLEDRIVER\0000"
pnputil /remove-device "ROOT\IDDSAMPLEDRIVER\0001"

echo.
echo ========================================
echo 清理完成！现在可以重新安装
echo ========================================
echo.

pause

