Write-Host "=== VDD Install/Uninstall Test ===" -ForegroundColor Cyan

Write-Host "`n[1] Install" -ForegroundColor Yellow
.\build\bin\Release\vddctl.exe install x64\Release\IddSampleDriver\IddSampleDriver.inf

Write-Host "`n[2] Check Device" -ForegroundColor Yellow
Get-PnpDevice | Where-Object { $_.InstanceId -like '*IddSampleDriver*' } | Format-Table -AutoSize

Write-Host "`n[3] Check ProblemCode" -ForegroundColor Yellow
$device = Get-PnpDevice | Where-Object { $_.InstanceId -like '*IddSampleDriver*' }
if ($device) {
    $prob = (Get-PnpDeviceProperty -InstanceId $device.InstanceId -KeyName 'DEVPKEY_Device_ProblemCode').Data
    if ($prob -eq 0) {
        Write-Host "ProblemCode: 0 (OK)" -ForegroundColor Green
    } else {
        Write-Host "ProblemCode: $prob (ERROR)" -ForegroundColor Red
    }
}

Write-Host "`n[4] Uninstall" -ForegroundColor Yellow
.\build\bin\Release\vddctl.exe uninstall

Write-Host "`n[5] Verify Removal" -ForegroundColor Yellow
$d = Get-PnpDevice | Where-Object { $_.InstanceId -like '*IddSampleDriver*' }
if ($d) { 
    Write-Host "FAILED: Device still present" -ForegroundColor Red 
    $d | Format-Table -AutoSize
} else { 
    Write-Host "SUCCESS: Device removed" -ForegroundColor Green 
}



