@echo off
echo ========================================
echo 测试最佳方法（带回滚）
echo ========================================
echo.

REM 1. 先卸载
echo 步骤 1: 卸载现有设备...
powershell -Command "Start-Process -FilePath '%CD%\uninstall_driver.exe' -Verb RunAs -Wait"
echo.

REM 2. 安装新方法
echo 步骤 2: 使用新方法安装...
powershell -Command "Start-Process -FilePath '%CD%\install_best.exe' -Verb RunAs -Wait"
echo.

REM 3. 验证结果
echo 步骤 3: 验证安装结果...
powershell -Command "Get-PnpDevice | Where-Object { $_.FriendlyName -like '*IddSampleDriver*' } | Format-Table FriendlyName, Status, InstanceId, Class, ClassGuid -AutoSize"
echo.

echo ========================================
echo 测试完成
echo ========================================
pause

