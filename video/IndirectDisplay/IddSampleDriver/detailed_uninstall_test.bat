@echo off
setlocal enabledelayedexpansion

set "VDDCTL=.\build\bin\Release\vddctl.exe"

echo ========================================
echo Detailed UninstallDriver Test
echo ========================================
echo.

:: Check admin
whoami /groups | find "S-1-5-32-544" > nul
if %errorlevel% neq 0 (
    echo ERROR: Not running as administrator!
    pause
    exit /b 1
)
echo Admin: OK
echo.

echo ========================================
echo Step 1: Count devices BEFORE
echo ========================================
powershell -Command "$count = (Get-PnpDevice -Class Display | Where-Object { $_.InstanceId -like '*Idd*' }).Count; Write-Host \"Found $count IddSampleDriver devices\"; Get-PnpDevice -Class Display | Where-Object { $_.InstanceId -like '*Idd*' } | Format-Table FriendlyName, Status, InstanceId -AutoSize"
echo.
pause

echo ========================================
echo Step 2: Run vddctl uninstall
echo ========================================
"%VDDCTL%" uninstall > uninstall_log.txt 2>&1
type uninstall_log.txt
echo.
pause

echo ========================================
echo Step 3: Count devices AFTER
echo ========================================
powershell -Command "$count = (Get-PnpDevice -Class Display | Where-Object { $_.InstanceId -like '*Idd*' }).Count; Write-Host \"Found $count IddSampleDriver devices\"; Get-PnpDevice -Class Display | Where-Object { $_.InstanceId -like '*Idd*' } | Format-Table FriendlyName, Status, InstanceId -AutoSize"
echo.
pause

echo ========================================
echo Step 4: Check if DIF_REMOVE actually worked
echo ========================================
echo Checking Device Manager status...
powershell -Command "Get-PnpDevice -Class Display | Where-Object { $_.InstanceId -like '*Idd*' } | ForEach-Object { Write-Host \"Device: $($_.FriendlyName)\"; Write-Host \"  Status: $($_.Status)\"; Write-Host \"  ProblemCode: $($_.Problem)\"; Write-Host \"  ConfigManagerErrorCode: $($_.ConfigManagerErrorCode)\"; Write-Host \"\" }"
echo.

echo ========================================
echo Step 5: Try manual device removal
echo ========================================
echo Attempting PowerShell device removal...
powershell -Command "$devices = Get-PnpDevice -Class Display | Where-Object { $_.InstanceId -like '*Idd*' }; foreach ($dev in $devices) { Write-Host \"Removing: $($dev.InstanceId)\"; try { $dev | Disable-PnpDevice -Confirm:$false -ErrorAction Stop; $dev | Remove-PnpDevice -Confirm:$false -ErrorAction Stop; Write-Host \"  SUCCESS\" -ForegroundColor Green; } catch { Write-Host \"  FAILED: $_\" -ForegroundColor Red; } }"
echo.
pause

echo ========================================
echo Step 6: Final verification
echo ========================================
powershell -Command "$count = (Get-PnpDevice -Class Display | Where-Object { $_.InstanceId -like '*Idd*' }).Count; if ($count -eq 0) { Write-Host \"SUCCESS: All devices removed!\" -ForegroundColor Green; } else { Write-Host \"FAILED: Still have $count device(s)\" -ForegroundColor Red; Get-PnpDevice -Class Display | Where-Object { $_.InstanceId -like '*Idd*' } | Format-Table FriendlyName, Status, InstanceId -AutoSize }"
echo.

echo ========================================
echo Test Complete
echo ========================================
pause
endlocal

