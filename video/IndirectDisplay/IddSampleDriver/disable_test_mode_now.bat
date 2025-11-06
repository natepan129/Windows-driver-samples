@echo off
echo ========================================
echo 关闭测试模式
echo ========================================
echo.

REM 检查管理员权限
net session >nul 2>&1
if %errorLevel% NEQ 0 (
    echo [错误] 需要管理员权限！
    echo.
    echo 请右键点击此文件，选择"以管理员身份运行"
    echo.
    pause
    exit /b 1
)

echo 正在关闭测试模式...
bcdedit /set testsigning off

if %errorLevel% EQU 0 (
    echo.
    echo ========================================
    echo [成功] 测试模式已关闭！
    echo ========================================
    echo.
    echo 重启后，桌面右下角的"测试模式"水印会消失
    echo.
    echo 注意：关闭测试模式后，未签名的驱动将无法加载
    echo.
    choice /C YN /M "现在重启电脑吗？(Y=是, N=否)"
    if errorlevel 2 goto :no_reboot
    
    echo.
    echo 3 秒后重启...
    timeout /t 3 /nobreak
    shutdown /r /t 0
    
) else (
    echo.
    echo [错误] 无法关闭测试模式！
    echo 请确认以管理员身份运行
    echo.
)
goto :end

:no_reboot
echo.
echo 请手动重启电脑：shutdown /r /t 0
echo.

:end
pause







