@echo off
net session >nul 2>&1
if %errorlevel% neq 0 (
    echo ERROR: Run as Administrator!
    pause
    exit /b 1
)

echo Uninstalling...
.\build\bin\Release\vddctl.exe uninstall

echo.
echo Checking cleanup...
powershell -Command "$d = Get-PnpDevice | Where-Object { $_.InstanceId -like '*IddSampleDriver*' }; if ($d) { Write-Host 'Device still present:' -ForegroundColor Yellow; $d | Format-Table -AutoSize } else { Write-Host 'SUCCESS: Completely removed' -ForegroundColor Green }"

pause

