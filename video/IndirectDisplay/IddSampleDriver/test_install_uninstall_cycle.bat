@echo off
chcp 65001 >nul
echo ========================================
echo 完整测试：安装 → 验证 → 卸载 → 验证
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

echo [步骤 1] 清理现有设备（如果有）
echo ========================================
pnputil /remove-device "ROOT\IDDSAMPLEDRIVER\0000" 2>nul
pnputil /remove-device "ROOT\IDDSAMPLEDRIVER\0001" 2>nul
echo 清理完成
echo.
timeout /t 2 >nul

echo [步骤 2] 安装驱动
echo ========================================
if not exist install_driver.exe (
    echo 编译 install_driver.exe...
    call compile_and_install.bat >nul 2>&1
)

echo 运行安装...
install_driver.exe IddSampleDriver_Fixed.inf
echo.
timeout /t 2 >nul

echo [步骤 3] 验证安装
echo ========================================
powershell -Command "& { $d = Get-PnpDevice | Where-Object { $_.InstanceId -like '*IddSampleDriver*' }; if ($d) { Write-Host '安装成功！找到设备:' -ForegroundColor Green; $d | Format-Table FriendlyName, Status, Class -AutoSize } else { Write-Host '安装失败：未找到设备' -ForegroundColor Red } }"
echo.
echo 按任意键继续测试卸载...
pause

echo.
echo [步骤 4] 卸载驱动
echo ========================================
if not exist uninstall_driver.exe (
    echo 编译 uninstall_driver.exe...
    call compile_and_uninstall.bat >nul 2>&1
)

echo 运行卸载...
uninstall_driver.exe
echo.
timeout /t 2 >nul

echo [步骤 5] 验证卸载
echo ========================================
powershell -Command "& { $d = Get-PnpDevice | Where-Object { $_.InstanceId -like '*IddSampleDriver*' }; if ($d) { Write-Host '卸载失败：仍有设备存在' -ForegroundColor Red; $d | Format-Table FriendlyName, Status, Class -AutoSize } else { Write-Host '卸载成功！所有设备已移除' -ForegroundColor Green } }"

echo.
echo ========================================
echo 测试完成！
echo ========================================
echo.
echo 测试摘要:
echo   [1] 清理 - 完成
echo   [2] 安装 - 请查看上面的结果
echo   [3] 验证安装 - 请查看上面的结果
echo   [4] 卸载 - 请查看上面的结果
echo   [5] 验证卸载 - 请查看上面的结果
echo.
pause

