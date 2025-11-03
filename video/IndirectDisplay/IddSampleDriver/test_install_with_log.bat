@echo off
cd /d "%~dp0"
echo Installing and checking status...
build\bin\Release\vddctl.exe install --inf "%CD%\IddSampleDriver_Fixed.inf" > install_log.txt 2>&1
echo.
echo Install log:
type install_log.txt
echo.
echo.
echo Device status:
powershell -Command "Get-PnpDevice | Where-Object { $_.InstanceId -like '*IddSampleDriver*' } | Format-Table FriendlyName, Status, InstanceId -AutoSize"
pause

