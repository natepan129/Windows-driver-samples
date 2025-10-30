@echo off
chcp 65001 >nul
echo ========================================
echo 完整测试修复后的 INF
echo ========================================
echo.

REM 检查管理员权限
net session >nul 2>&1
if %errorLevel% neq 0 (
    echo 错误：需要管理员权限！
    echo 请右键此脚本，选择"以管理员身份运行"
    echo.
    pause
    exit /b 1
)

echo [步骤 1] 清理所有旧设备
echo ----------------------------------------
pnputil /remove-device "ROOT\IDDSAMPLEDRIVER\0000" 2>nul
pnputil /remove-device "ROOT\IDDSAMPLEDRIVER\0001" 2>nul
echo 旧设备已清理

echo.
echo [步骤 2] 使用修复后的 INF 安装
echo ----------------------------------------

REM 检查 install_driver.exe 是否存在
if not exist install_driver.exe (
    echo 编译 install_driver.exe...
    call compile_and_install.bat
) else (
    echo 使用现有的 install_driver.exe
    install_driver.exe IddSampleDriver_Fixed.inf
)

echo.
echo [步骤 3] 验证安装结果
echo ----------------------------------------
timeout /t 2 >nul

powershell -Command "& { $devices = Get-PnpDevice | Where-Object { $_.InstanceId -like '*IddSampleDriver*' }; if ($devices) { Write-Host ''; Write-Host '找到设备:'; Write-Host ''; $devices | Format-Table FriendlyName, Status, Class, ClassGuid, InstanceId -AutoSize; Write-Host ''; $withClass = ($devices | Where-Object { $_.Class -eq 'Display' }).Count; $total = $devices.Count; if ($withClass -gt 0) { Write-Host '[成功] 有 Display Class 的设备数量:' $withClass '/' $total -ForegroundColor Green } else { Write-Host '[失败] 所有设备都没有 Display Class' -ForegroundColor Red } } else { Write-Host '[失败] 未找到任何设备' -ForegroundColor Red } }"

echo.
echo ========================================
echo 测试完成
echo ========================================
pause

