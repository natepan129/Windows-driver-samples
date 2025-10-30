@echo off
echo ========================================
echo 运行智能回滚安装工具
echo ========================================
echo.
echo 注意：这会提示 UAC，请点击"是"授权
echo.
pause

REM 运行安装程序
install_driver_smart.exe IddSampleDriver_Fixed.inf

echo.
echo ========================================
echo 安装程序执行完成
echo ========================================
echo.
echo 按任意键检查设备状态...
pause

REM 检查设备
powershell -Command "& { $devices = Get-PnpDevice | Where-Object { $_.InstanceId -like '*IddSampleDriver*' }; if ($devices) { Write-Host ''; Write-Host '找到以下设备:'; $devices | Format-Table FriendlyName, Status, Class, ClassGuid, InstanceId -AutoSize } else { Write-Host ''; Write-Host '未找到 IddSampleDriver 设备' } }"

echo.
pause

