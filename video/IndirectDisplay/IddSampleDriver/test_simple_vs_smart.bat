@echo off
chcp 65001 >nul
echo ========================================
echo 对比测试：简单版 vs 智能回滚版
echo ========================================
echo.

echo [测试 1] 使用原始简单版本（已验证可用）
echo ========================================
echo.

if not exist install_driver.exe (
    echo 错误：install_driver.exe 不存在，先编译...
    pause
    exit /b 1
)

echo 运行 install_driver.exe...
install_driver.exe IddSampleDriver_Fixed.inf
echo.
echo 安装完成！检查设备...
echo.

powershell -Command "& { $d = Get-PnpDevice | Where-Object { $_.InstanceId -like '*IddSampleDriver*' }; if ($d) { $d | Format-Table FriendlyName, Status, Class -AutoSize } else { Write-Host '未找到设备' } }"

echo.
echo ========================================
echo 按任意键继续测试智能版本...
pause

echo.
echo [测试 2] 测试智能回滚版本
echo ========================================
echo.

if exist install_driver_smart.exe (
    echo 运行 install_driver_smart.exe...
    install_driver_smart.exe IddSampleDriver_Fixed.inf
    echo.
    echo 安装完成！检查设备...
    echo.
    
    powershell -Command "& { $d = Get-PnpDevice | Where-Object { $_.InstanceId -like '*IddSampleDriver*' }; if ($d) { $d | Format-Table FriendlyName, Status, Class -AutoSize } else { Write-Host '未找到设备' } }"
) else (
    echo 错误：install_driver_smart.exe 不存在
)

echo.
pause

