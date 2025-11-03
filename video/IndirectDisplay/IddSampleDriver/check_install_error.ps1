# Check Installation Error Details

Write-Host "=== Checking Installation Failure ===" -ForegroundColor Cyan

Write-Host "`n[1] Testing Install" -ForegroundColor Yellow
.\build\bin\Release\vddctl.exe install "x64\Release\IddSampleDriver\IddSampleDriver.inf"

Write-Host "`n[2] Checking Device Manager" -ForegroundColor Yellow
$device = Get-PnpDevice | Where-Object { $_.InstanceId -like '*IddSampleDriver*' }
if ($device) {
    Write-Host "Device found:" -ForegroundColor Green
    $device | Format-Table Status, ProblemCode, Class, FriendlyName -AutoSize
    
    # Get detailed error
    $prob = (Get-PnpDeviceProperty -InstanceId $device.InstanceId -KeyName 'DEVPKEY_Device_ProblemCode').Data
    Write-Host "ProblemCode: $prob" -ForegroundColor $(if ($prob -eq 0) { "Green" } else { "Red" })
    
    if ($prob -ne 0) {
        $probDesc = (Get-PnpDeviceProperty -InstanceId $device.InstanceId -KeyName 'DEVPKEY_Device_ProblemStatus').Data
        Write-Host "Problem Description: $probDesc" -ForegroundColor Yellow
    }
} else {
    Write-Host "Device not found" -ForegroundColor Red
}

Write-Host "`n[3] Checking Setup API Logs" -ForegroundColor Yellow
$setupLog = "C:\Windows\INF\setupapi.dev.log"
if (Test-Path $setupLog) {
    Write-Host "Last 50 lines of setupapi.dev.log:" -ForegroundColor Yellow
    Get-Content $setupLog -Tail 50 | Select-String "IddSampleDriver" -Context 2,2
} else {
    Write-Host "Log not found: $setupLog" -ForegroundColor Red
}

Write-Host "`n[4] Checking Driver Store" -ForegroundColor Yellow
pnputil /enum-drivers | Select-String "IddSampleDriver" -Context 5



# Check Installation Error Details

Write-Host "=== Checking Installation Failure ===" -ForegroundColor Cyan

Write-Host "`n[1] Testing Install" -ForegroundColor Yellow
.\build\bin\Release\vddctl.exe install "x64\Release\IddSampleDriver\IddSampleDriver.inf"

Write-Host "`n[2] Checking Device Manager" -ForegroundColor Yellow
$device = Get-PnpDevice | Where-Object { $_.InstanceId -like '*IddSampleDriver*' }
if ($device) {
    Write-Host "Device found:" -ForegroundColor Green
    $device | Format-Table Status, ProblemCode, Class, FriendlyName -AutoSize
    
    # Get detailed error
    $prob = (Get-PnpDeviceProperty -InstanceId $device.InstanceId -KeyName 'DEVPKEY_Device_ProblemCode').Data
    Write-Host "ProblemCode: $prob" -ForegroundColor $(if ($prob -eq 0) { "Green" } else { "Red" })
    
    if ($prob -ne 0) {
        $probDesc = (Get-PnpDeviceProperty -InstanceId $device.InstanceId -KeyName 'DEVPKEY_Device_ProblemStatus').Data
        Write-Host "Problem Description: $probDesc" -ForegroundColor Yellow
    }
} else {
    Write-Host "Device not found" -ForegroundColor Red
}

Write-Host "`n[3] Checking Setup API Logs" -ForegroundColor Yellow
$setupLog = "C:\Windows\INF\setupapi.dev.log"
if (Test-Path $setupLog) {
    Write-Host "Last 50 lines of setupapi.dev.log:" -ForegroundColor Yellow
    Get-Content $setupLog -Tail 50 | Select-String "IddSampleDriver" -Context 2,2
} else {
    Write-Host "Log not found: $setupLog" -ForegroundColor Red
}

Write-Host "`n[4] Checking Driver Store" -ForegroundColor Yellow
pnputil /enum-drivers | Select-String "IddSampleDriver" -Context 5



