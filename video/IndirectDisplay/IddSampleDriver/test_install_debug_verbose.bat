@echo off
cd /d "%~dp0"
echo Testing vddctl install with verbose output...
echo ========================================
echo.

build\bin\Release\vddctl.exe install --inf "%CD%\IddSampleDriver.inf" > install_debug.txt 2>&1
echo Exit code: %ERRORLEVEL%
echo.
echo Output:
type install_debug.txt
echo.
echo ========================================
echo Checking devices...
powershell -Command "Get-PnpDevice | Where-Object { $_.InstanceId -like '*IddSampleDriver*' } | Format-Table"
pause

