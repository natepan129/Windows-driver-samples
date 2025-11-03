Write-Host "========================================" -ForegroundColor Cyan
Write-Host "   VDD SDK Complete Command Test" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan

$vddctl = ".\build\bin\Release\vddctl.exe"
$inf = "x64\Release\IddSampleDriver\IddSampleDriver.inf"

Write-Host "`n[1] Version" -ForegroundColor Yellow
& $vddctl version

Write-Host "`n[2] Init" -ForegroundColor Yellow
& $vddctl init

Write-Host "`n[3] Status (before install)" -ForegroundColor Yellow
& $vddctl status

Write-Host "`n[4] List (before install)" -ForegroundColor Yellow
& $vddctl list

Write-Host "`n[5] Install" -ForegroundColor Yellow
& $vddctl install $inf

Write-Host "`n[6] Status (after install)" -ForegroundColor Yellow
& $vddctl status

Write-Host "`n[7] List (after install)" -ForegroundColor Yellow
& $vddctl list

Write-Host "`n[8] Check Device (PowerShell)" -ForegroundColor Yellow
$device = Get-PnpDevice | Where-Object { $_.InstanceId -like '*IddSampleDriver*' }
if ($device) {
    Write-Host "Device found:" -ForegroundColor Green
    $device | Format-Table Status, Class, FriendlyName -AutoSize
    $prob = (Get-PnpDeviceProperty -InstanceId $device.InstanceId -KeyName 'DEVPKEY_Device_ProblemCode').Data
    if ($prob -eq 0) {
        Write-Host "ProblemCode: 0 (OK)" -ForegroundColor Green
    } else {
        Write-Host "ProblemCode: $prob (ERROR)" -ForegroundColor Red
    }
} else {
    Write-Host "Device not found" -ForegroundColor Red
}

Write-Host "`n[9] Uninstall" -ForegroundColor Yellow
& $vddctl uninstall

Write-Host "`n[10] Status (after uninstall)" -ForegroundColor Yellow
& $vddctl status

Write-Host "`n[11] Verify Removal" -ForegroundColor Yellow
$device = Get-PnpDevice | Where-Object { $_.InstanceId -like '*IddSampleDriver*' }
if ($device) {
    Write-Host "FAILED: Device still present" -ForegroundColor Red
    $device | Format-Table -AutoSize
} else {
    Write-Host "SUCCESS: Device completely removed" -ForegroundColor Green
}

Write-Host "`n[12] Shutdown" -ForegroundColor Yellow
& $vddctl shutdown

Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host "   Test Complete!" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan

Write-Host "   VDD SDK Complete Command Test" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan

$vddctl = ".\build\bin\Release\vddctl.exe"
$inf = "x64\Release\IddSampleDriver\IddSampleDriver.inf"

Write-Host "`n[1] Version" -ForegroundColor Yellow
& $vddctl version

Write-Host "`n[2] Init" -ForegroundColor Yellow
& $vddctl init

Write-Host "`n[3] Status (before install)" -ForegroundColor Yellow
& $vddctl status

Write-Host "`n[4] List (before install)" -ForegroundColor Yellow
& $vddctl list

Write-Host "`n[5] Install" -ForegroundColor Yellow
& $vddctl install $inf

Write-Host "`n[6] Status (after install)" -ForegroundColor Yellow
& $vddctl status

Write-Host "`n[7] List (after install)" -ForegroundColor Yellow
& $vddctl list

Write-Host "`n[8] Check Device (PowerShell)" -ForegroundColor Yellow
$device = Get-PnpDevice | Where-Object { $_.InstanceId -like '*IddSampleDriver*' }
if ($device) {
    Write-Host "Device found:" -ForegroundColor Green
    $device | Format-Table Status, Class, FriendlyName -AutoSize
    $prob = (Get-PnpDeviceProperty -InstanceId $device.InstanceId -KeyName 'DEVPKEY_Device_ProblemCode').Data
    if ($prob -eq 0) {
        Write-Host "ProblemCode: 0 (OK)" -ForegroundColor Green
    } else {
        Write-Host "ProblemCode: $prob (ERROR)" -ForegroundColor Red
    }
} else {
    Write-Host "Device not found" -ForegroundColor Red
}

Write-Host "`n[9] Uninstall" -ForegroundColor Yellow
& $vddctl uninstall

Write-Host "`n[10] Status (after uninstall)" -ForegroundColor Yellow
& $vddctl status

Write-Host "`n[11] Verify Removal" -ForegroundColor Yellow
$device = Get-PnpDevice | Where-Object { $_.InstanceId -like '*IddSampleDriver*' }
if ($device) {
    Write-Host "FAILED: Device still present" -ForegroundColor Red
    $device | Format-Table -AutoSize
} else {
    Write-Host "SUCCESS: Device completely removed" -ForegroundColor Green
}

Write-Host "`n[12] Shutdown" -ForegroundColor Yellow
& $vddctl shutdown

Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host "   Test Complete!" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan



