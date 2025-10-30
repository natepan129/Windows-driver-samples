@echo off
chcp 65001 >nul
echo ========================================
echo 手动测试安装和卸载功能
echo ========================================
echo.
echo 此脚本会引导你完成测试步骤
echo 每个步骤需要以管理员权限运行对应的工具
echo.
pause

echo.
echo ========================================
echo 步骤 1: 安装驱动
echo ========================================
echo.
echo 即将运行: install_driver.exe IddSampleDriver_Fixed.inf
echo 请在 UAC 提示时点击"是"
echo.
pause

install_driver.exe IddSampleDriver_Fixed.inf

echo.
echo ========================================
echo 步骤 2: 验证安装
echo ========================================
echo.
timeout /t 2 >nul

powershell -Command "& { $d = Get-PnpDevice | Where-Object { $_.InstanceId -like '*IddSampleDriver*' }; if ($d) { Write-Host ''; Write-Host '安装成功！找到以下设备:' -ForegroundColor Green; Write-Host ''; $d | Format-Table FriendlyName, Status, Class, InstanceId -AutoSize; $withClass = ($d | Where-Object { $_.Class -eq 'Display' }).Count; Write-Host ''; Write-Host '有 Display Class 的设备:' $withClass '/' $d.Count } else { Write-Host ''; Write-Host '安装失败：未找到设备' -ForegroundColor Red } }"

echo.
echo 按任意键继续测试卸载...
pause

echo.
echo ========================================
echo 步骤 3: 卸载驱动
echo ========================================
echo.
echo 即将运行: uninstall_driver.exe
echo 请在 UAC 提示时点击"是"
echo.
pause

uninstall_driver.exe

echo.
echo ========================================
echo 步骤 4: 验证卸载
echo ========================================
echo.
timeout /t 2 >nul

powershell -Command "& { $d = Get-PnpDevice | Where-Object { $_.InstanceId -like '*IddSampleDriver*' }; if ($d) { Write-Host ''; Write-Host '卸载失败：仍有以下设备存在' -ForegroundColor Red; Write-Host ''; $d | Format-Table FriendlyName, Status, Class, InstanceId -AutoSize } else { Write-Host ''; Write-Host '卸载成功！所有设备已移除' -ForegroundColor Green } }"

echo.
echo ========================================
echo 测试完成！
echo ========================================
echo.
pause

