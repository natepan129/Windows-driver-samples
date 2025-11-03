@echo off
:: Test Install with Correct INF Path
echo ========================================
echo Testing Install with MSBuild INF
echo ========================================

:: Check admin
net session >nul 2>&1
if %errorlevel% neq 0 (
    echo ERROR: Run as Administrator!
    pause
    exit /b 1
)

echo.
echo [1] Installing with MSBuild INF (CORRECT PATH)
echo ----------------------------------------
echo INF: x64\Release\IddSampleDriver\IddSampleDriver.inf
echo.

.\build\bin\Release\vddctl.exe install "x64\Release\IddSampleDriver\IddSampleDriver.inf"

echo.
echo [2] Check Device
echo ----------------------------------------
powershell -Command "$d = Get-PnpDevice | Where-Object { $_.InstanceId -like '*IddSampleDriver*' }; if ($d) { Write-Host 'SUCCESS: Device installed' -ForegroundColor Green; $d | Format-Table -AutoSize; $prob = (Get-PnpDeviceProperty -InstanceId $d.InstanceId -KeyName 'DEVPKEY_Device_ProblemCode').Data; Write-Host 'ProblemCode:' $prob -ForegroundColor $(if ($prob -eq 0) { 'Green' } else { 'Red' }) } else { Write-Host 'FAILED: Device not found' -ForegroundColor Red }"

pause



@echo off
:: Test Install with Correct INF Path
echo ========================================
echo Testing Install with MSBuild INF
echo ========================================

:: Check admin
net session >nul 2>&1
if %errorlevel% neq 0 (
    echo ERROR: Run as Administrator!
    pause
    exit /b 1
)

echo.
echo [1] Installing with MSBuild INF (CORRECT PATH)
echo ----------------------------------------
echo INF: x64\Release\IddSampleDriver\IddSampleDriver.inf
echo.

.\build\bin\Release\vddctl.exe install "x64\Release\IddSampleDriver\IddSampleDriver.inf"

echo.
echo [2] Check Device
echo ----------------------------------------
powershell -Command "$d = Get-PnpDevice | Where-Object { $_.InstanceId -like '*IddSampleDriver*' }; if ($d) { Write-Host 'SUCCESS: Device installed' -ForegroundColor Green; $d | Format-Table -AutoSize; $prob = (Get-PnpDeviceProperty -InstanceId $d.InstanceId -KeyName 'DEVPKEY_Device_ProblemCode').Data; Write-Host 'ProblemCode:' $prob -ForegroundColor $(if ($prob -eq 0) { 'Green' } else { 'Red' }) } else { Write-Host 'FAILED: Device not found' -ForegroundColor Red }"

pause



