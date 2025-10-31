@echo off
echo ========================================
echo Testing vddctl uninstall
echo ========================================
echo.

cd /d "%~dp0"

echo Running vddctl uninstall...
echo ----------------------------------------
build\bin\Release\vddctl.exe uninstall

echo.
echo ----------------------------------------
echo Checking remaining devices...
powershell -Command "Get-PnpDevice | Where-Object { $_.InstanceId -like '*IddSampleDriver*' } | Format-Table FriendlyName, Status, InstanceId, Class -AutoSize"

echo.
echo ========================================
echo Test Complete
echo ========================================
pause

