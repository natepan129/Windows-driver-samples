@echo off
REM ========================================
REM 禁用 Windows 测试模式
REM ========================================

echo.
echo ========================================
echo 禁用 Windows 测试模式
echo ========================================
echo.
echo 警告：禁用测试模式后，未签名的驱动将无法加载
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
echo 正在禁用测试模式...
bcdedit /set testsigning off

if %errorLevel% EQU 0 (
    echo.
    echo ========================================
    echo [成功] 测试模式已禁用！
    echo ========================================
    echo.
    echo 重启后"测试模式"水印会消失
    echo.
    
    choice /C YN /M "现在重启电脑吗？(Y=是, N=否)"
    if errorlevel 2 goto :manual_reboot
    
    echo.
    echo 5 秒后重启...
    shutdown /r /t 5 /c "禁用测试模式，重启生效"
    goto :end
    
:manual_reboot
    echo.
    echo ========================================
    echo 请手动重启电脑使设置生效
    echo 重启命令: shutdown /r /t 0
    echo ========================================
    
) else (
    echo.
    echo ========================================
    echo [错误] 无法禁用测试模式
    echo ========================================
    echo.
)

:end
pause





