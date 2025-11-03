@echo off
echo ==========================================
echo DEBUG UNINSTALL TEST
echo ==========================================
echo.

echo 1. Devices BEFORE uninstall:
powershell -Command "Get-PnpDevice | Where-Object { $_.InstanceId -like '*IddSampleDriver*' } | Select-Object FriendlyName, Status, InstanceId, ProblemCode | Format-Table -AutoSize"
echo.

echo 2. Registry check:
reg query "HKLM\SYSTEM\CurrentControlSet\Enum\ROOT\IddSampleDriver" 2>nul
if errorlevel 1 (
    echo Registry key NOT FOUND
) else (
    echo Registry key FOUND
)
echo.

echo 3. Running uninstall with verbose output...
.\build\bin\Release\vddctl.exe uninstall
echo.

echo 4. Devices AFTER uninstall:
powershell -Command "Get-PnpDevice | Where-Object { $_.InstanceId -like '*IddSampleDriver*' } | Select-Object FriendlyName, Status, InstanceId, ProblemCode | Format-Table -AutoSize"
echo.

echo 5. Registry check after uninstall:
reg query "HKLM\SYSTEM\CurrentControlSet\Enum\ROOT\IddSampleDriver" 2>nul
if errorlevel 1 (
    echo Registry key NOT FOUND
) else (
    echo Registry key FOUND
)
echo.

pause

