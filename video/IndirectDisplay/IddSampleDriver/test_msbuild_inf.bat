@echo off
:: Test MSBuild Generated INF (x64\Release)
echo ========================================
echo Testing MSBuild INF (WITHOUT IndirectKmd.sys in SourceDisksFiles)
echo ========================================

:: Check admin
net session >nul 2>&1
if %errorlevel% neq 0 (
    echo ERROR: Not running as Administrator!
    echo Right-click this file and select "Run as administrator"
    pause
    exit /b 1
)

echo.
echo [INFO] Using: x64\Release\IddSampleDriver\IddSampleDriver.inf
echo [INFO] This INF does NOT declare IndirectKmd.sys (system handles it via WUDFRD.inf)
echo.

echo [STEP 1] Check current state
echo ----------------------------------------
powershell -Command "Get-PnpDevice | Where-Object { $_.InstanceId -like '*IddSampleDriver*' } | Format-Table -AutoSize"

echo.
echo [STEP 2] Install using MSBuild INF
echo ----------------------------------------
.\build\bin\Release\vddctl.exe install x64\Release\IddSampleDriver\IddSampleDriver.inf
if %errorlevel% neq 0 (
    echo FAILED: Install failed
    pause
    exit /b 1
)

echo.
echo [STEP 3] Verify device
echo ----------------------------------------
powershell -Command "$d = Get-PnpDevice | Where-Object { $_.InstanceId -like '*IddSampleDriver*' }; if ($d) { Write-Host 'SUCCESS: Device installed' -ForegroundColor Green; $d | Format-Table -AutoSize } else { Write-Host 'FAILED: Device not found' -ForegroundColor Red; exit 1 }"

echo.
echo [STEP 4] Check device status
echo ----------------------------------------
powershell -Command "Get-PnpDevice | Where-Object { $_.InstanceId -like '*IddSampleDriver*' } | ForEach-Object { Write-Host 'Status:' $_.Status; Write-Host 'ProblemCode:' (Get-PnpDeviceProperty -InstanceId $_.InstanceId -KeyName 'DEVPKEY_Device_ProblemCode').Data }"

echo.
echo [STEP 5] Uninstall
echo ----------------------------------------
.\build\bin\Release\vddctl.exe uninstall

echo.
echo [STEP 6] Verify cleanup
echo ----------------------------------------
powershell -Command "$d = Get-PnpDevice | Where-Object { $_.InstanceId -like '*IddSampleDriver*' }; if ($d) { Write-Host 'Device still present:' -ForegroundColor Yellow; $d | Format-Table -AutoSize } else { Write-Host 'SUCCESS: Completely removed' -ForegroundColor Green }"

echo.
echo ========================================
echo TEST COMPLETED
echo ========================================
pause

