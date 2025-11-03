@echo off
net session >nul 2>&1
if %errorlevel% neq 0 (
    echo ERROR: Run as Administrator!
    pause
    exit /b 1
)

echo Installing with MSBuild INF...
.\build\bin\Release\vddctl.exe install x64\Release\IddSampleDriver\IddSampleDriver.inf

echo.
echo Checking device...
powershell -Command "Get-PnpDevice | Where-Object { $_.InstanceId -like '*IddSampleDriver*' } | Format-Table -AutoSize"

echo.
echo Checking ProblemCode...
powershell -Command "$d = Get-PnpDevice | Where-Object { $_.InstanceId -like '*IddSampleDriver*' } | Select-Object -First 1; if ($d) { $prob = (Get-PnpDeviceProperty -InstanceId $d.InstanceId -KeyName 'DEVPKEY_Device_ProblemCode').Data; Write-Host 'ProblemCode:' $prob -ForegroundColor $(if ($prob -eq 0) { 'Green' } else { 'Red' }) }"

pause

