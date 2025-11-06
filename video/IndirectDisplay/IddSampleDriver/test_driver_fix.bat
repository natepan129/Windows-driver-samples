@echo off
REM ========================================
REM 测试修复后的虚拟显示驱动
REM ========================================

echo.
echo ========================================
echo 虚拟显示驱动修复测试脚本
echo ========================================
echo.

REM 检查管理员权限
net session >nul 2>&1
if %errorLevel% NEQ 0 (
    echo [错误] 需要管理员权限！
    echo 请右键选择 "以管理员身份运行"
    echo.
    pause
    exit /b 1
)

echo [✓] 管理员权限确认
echo.

REM 设置工作目录
cd /d "%~dp0"

REM 步骤 1: 卸载旧驱动
echo ========================================
echo 步骤 1/5: 卸载旧驱动
echo ========================================
.\vddctl.exe uninstall
echo.
timeout /t 2 /nobreak >nul

REM 步骤 2: 安装新驱动
echo ========================================
echo 步骤 2/5: 安装修复后的驱动
echo ========================================
.\vddctl.exe install .\x64\Release\IddSampleDriver.inf
if %errorLevel% NEQ 0 (
    echo [错误] 驱动安装失败！
    pause
    exit /b 1
)
echo.
timeout /t 2 /nobreak >nul

REM 步骤 3: 检查安装状态
echo ========================================
echo 步骤 3/5: 检查安装状态
echo ========================================
.\vddctl.exe status
echo.
timeout /t 2 /nobreak >nul

REM 步骤 4: 激活虚拟显示
echo ========================================
echo 步骤 4/5: 激活虚拟显示 (1920x1080)
echo ========================================
.\vddctl.exe activate --width 1920 --height 1080 --count 1
if %errorLevel% NEQ 0 (
    echo [错误] 激活失败！
    pause
    exit /b 1
)
echo.
timeout /t 2 /nobreak >nul

REM 步骤 5: 列出显示设备
echo ========================================
echo 步骤 5/5: 列出所有显示设备
echo ========================================
.\vddctl.exe list
echo.

REM 最终状态检查
echo ========================================
echo 最终状态
echo ========================================
.\vddctl.exe status
echo.

echo ========================================
echo 测试完成！
echo ========================================
echo.
echo 请检查：
echo 1. Windows 显示设置中是否出现虚拟显示器
echo 2. OBS 中是否能看到 "IddSampleDriver" 显示器
echo 3. 虚拟显示是否不再灰色显示
echo.
echo 按任意键退出...
pause >nul








