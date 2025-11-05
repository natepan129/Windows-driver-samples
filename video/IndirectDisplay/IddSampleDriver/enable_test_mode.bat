@echo off
REM ========================================
REM 启用 Windows 测试模式
REM ========================================

echo.
echo ========================================
echo 启用 Windows 测试模式
echo ========================================
echo.
echo 测试模式允许安装未签名的驱动程序
echo 桌面右下角会显示"测试模式"水印
echo.
echo 重要提示：
echo - 仅用于开发和测试环境
echo - 需要重启电脑才能生效
echo - 可随时关闭测试模式
echo.
pause

REM 检查管理员权限
net session >nul 2>&1
if %errorLevel% NEQ 0 (
    echo.
    echo [错误] 需要管理员权限！
    echo 请右键选择 "以管理员身份运行"
    echo.
    pause
    exit /b 1
)

echo.
echo 正在启用测试模式...
bcdedit /set testsigning on

if %errorLevel% EQU 0 (
    echo.
    echo ========================================
    echo [成功] 测试模式已启用！
    echo ========================================
    echo.
    echo 重启后会在桌面右下角显示"测试模式"水印
    echo.
    
    choice /C YN /M "现在重启电脑吗？(Y=是, N=否)"
    if errorlevel 2 goto :manual_reboot
    
    echo.
    echo 5 秒后重启...
    shutdown /r /t 5 /c "启用测试模式，重启生效"
    goto :end
    
:manual_reboot
    echo.
    echo ========================================
    echo 请手动重启电脑使测试模式生效
    echo 重启命令: shutdown /r /t 0
    echo ========================================
    
) else (
    echo.
    echo ========================================
    echo [错误] 无法启用测试模式
    echo ========================================
    echo.
    echo 可能的原因：
    echo 1. 未以管理员身份运行
    echo 2. UEFI 安全启动已启用（需要在 BIOS 中禁用）
    echo 3. 系统策略禁止测试模式
    echo.
)

:end
pause




