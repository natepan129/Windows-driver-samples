@echo off
echo ========================================
echo Check IddSampleDriver Device Status
echo ========================================
echo.

powershell -Command "& { $devices = Get-PnpDevice | Where-Object { $_.InstanceId -like '*IddSampleDriver*' }; if ($devices) { $devices | Format-Table FriendlyName, Status, Class, ClassGuid, InstanceId -AutoSize } else { Write-Host 'No IddSampleDriver devices found' } }"

echo.
pause
