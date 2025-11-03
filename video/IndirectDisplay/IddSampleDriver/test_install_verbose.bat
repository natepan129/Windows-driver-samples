@echo off
cd /d "%~dp0"
build\bin\Release\vddctl.exe install --inf "%CD%\IddSampleDriver_Fixed.inf" > install_verbose.txt 2>&1
type install_verbose.txt
echo.
echo ----------------------------------------
echo Checking devices...
powershell -Command "Get-PnpDevice | Where-Object { $_.InstanceId -like '*IddSampleDriver*' } | Format-Table FriendlyName, Status, InstanceId, Class -AutoSize"
pause

