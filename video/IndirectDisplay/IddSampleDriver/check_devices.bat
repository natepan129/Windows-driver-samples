@echo off
echo ========================================
echo 检查 IddSampleDriver 设备状态
echo ========================================
echo.

powershell -Command "& { $devices = Get-PnpDevice | Where-Object { $_.InstanceId -like '*IddSampleDriver*' }; if ($devices) { $devices | Format-Table FriendlyName, Status, Class, ClassGuid, InstanceId -AutoSize } else { Write-Host '未找到 IddSampleDriver 设备' } }"

echo.
pause

