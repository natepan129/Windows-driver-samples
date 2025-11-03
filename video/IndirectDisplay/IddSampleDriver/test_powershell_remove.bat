@echo off
setlocal

echo ========================================
echo Test PowerShell Remove-PnpDevice
echo ========================================
echo.

:: Check admin
whoami /groups | find "S-1-5-32-544" > nul
if %errorlevel% neq 0 (
    echo ERROR: Administrator privileges required!
    pause
    exit /b 1
)
echo Admin: OK
echo.

echo Current devices BEFORE:
powershell -Command "Get-PnpDevice -Class Display | Where-Object { $_.InstanceId -like '*Idd*' } | Format-Table FriendlyName, Status, InstanceId -AutoSize"
echo.
pause

echo ========================================
echo Attempting PowerShell removal...
echo ========================================
powershell -Command "$devices = Get-PnpDevice -Class Display | Where-Object { $_.InstanceId -like '*Idd*' }; $count = $devices.Count; Write-Host \"Found $count devices to remove\"; foreach ($dev in $devices) { Write-Host \"Removing: $($dev.InstanceId)\"; try { Disable-PnpDevice -InstanceId $dev.InstanceId -Confirm:$false -ErrorAction Stop; Write-Host \"  Disabled\" -ForegroundColor Yellow; Remove-PnpDevice -InstanceId $dev.InstanceId -Confirm:$false -ErrorAction Stop; Write-Host \"  Removed\" -ForegroundColor Green; } catch { Write-Host \"  FAILED: $_\" -ForegroundColor Red; } }"
echo.
pause

echo ========================================
echo Devices AFTER:
echo ========================================
powershell -Command "$devices = Get-PnpDevice -Class Display | Where-Object { $_.InstanceId -like '*Idd*' }; if ($devices) { Write-Host 'Still have devices:' -ForegroundColor Red; $devices | Format-Table FriendlyName, Status, InstanceId -AutoSize } else { Write-Host 'SUCCESS: All devices removed!' -ForegroundColor Green }"
echo.

echo ========================================
echo Complete!
echo ========================================
pause
endlocal

