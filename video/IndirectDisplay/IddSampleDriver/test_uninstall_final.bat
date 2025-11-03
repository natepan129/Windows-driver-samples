@echo off
echo ========================================
echo Test Fixed UninstallDriver
echo ========================================
echo.

:: Check admin
whoami /groups | find "S-1-5-32-544" > nul
if %errorlevel% neq 0 (
    echo ERROR: Administrator privileges required!
    pause
    exit /b 1
)
echo Admin: OK
echo.

echo ========================================
echo Step 1: Rebuild vddsdk.dll and vddctl.exe
echo ========================================
cd build
"C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" VddSdk.sln /t:vddsdk;vddctl /p:Configuration=Release /p:Platform=x64 /v:minimal /nologo
if %errorlevel% neq 0 (
    echo Build failed!
    cd ..
    pause
    exit /b 1
)
cd ..
echo Build successful!
echo.

echo ========================================
echo Step 1.5: Check and copy built files
echo ========================================
echo.
echo Checking build output locations...
echo.

:: Check WDKRemoteUser path (incorrect path from previous issue)
if exist "C:\Users\WDKRemoteUser\Desktop\Windows-driver-samples\video\IndirectDisplay\IddSampleDriver\build\bin\Release\vddsdk.dll" (
    echo WARNING: Files found in WDKRemoteUser directory!
    echo Source: C:\Users\WDKRemoteUser\Desktop\...\build\bin\Release\
    echo Copying to current directory...
    xcopy /Y "C:\Users\WDKRemoteUser\Desktop\Windows-driver-samples\video\IndirectDisplay\IddSampleDriver\build\bin\Release\vddsdk.dll" ".\build\bin\Release\"
    xcopy /Y "C:\Users\WDKRemoteUser\Desktop\Windows-driver-samples\video\IndirectDisplay\IddSampleDriver\build\bin\Release\vddctl.exe" ".\build\bin\Release\"
    xcopy /Y "C:\Users\WDKRemoteUser\Desktop\Windows-driver-samples\video\IndirectDisplay\IddSampleDriver\build\lib\Release\vddsdk.lib" ".\build\lib\Release\"
    echo Files copied!
    echo.
)

:: Check if files exist in correct location
if exist ".\build\bin\Release\vddsdk.dll" (
    echo [OK] vddsdk.dll found in .\build\bin\Release\
    dir /TC ".\build\bin\Release\vddsdk.dll" | find "vddsdk.dll"
) else (
    echo [ERROR] vddsdk.dll NOT found in .\build\bin\Release\
)

if exist ".\build\bin\Release\vddctl.exe" (
    echo [OK] vddctl.exe found in .\build\bin\Release\
    dir /TC ".\build\bin\Release\vddctl.exe" | find "vddctl.exe"
) else (
    echo [ERROR] vddctl.exe NOT found in .\build\bin\Release\
)

echo.
echo Verifying files are recent (should be from last few minutes)...
powershell -Command "Get-ChildItem .\build\bin\Release\vddsdk.dll,.\build\bin\Release\vddctl.exe | Select-Object Name, LastWriteTime | Format-Table -AutoSize"
echo.
pause

echo ========================================
echo Step 2: Check initial device status
echo ========================================
powershell -Command "Get-PnpDevice | Where-Object { $_.InstanceId -like '*Idd*' } | Format-Table FriendlyName, Status, InstanceId -AutoSize"
echo.
pause

echo ========================================
echo Step 3: Install driver
echo ========================================
.\build\bin\Release\vddctl.exe install --inf x64\Release\IddSampleDriver\IddSampleDriver.inf
echo.
pause

echo ========================================
echo Step 4: Verify installation
echo ========================================
powershell -Command "Get-PnpDevice | Where-Object { $_.InstanceId -like '*Idd*' } | Format-Table FriendlyName, Status, InstanceId -AutoSize"
echo.
pause

echo ========================================
echo Step 5: Test uninstall
echo ========================================
.\build\bin\Release\vddctl.exe uninstall
echo.
pause

echo ========================================
echo Step 6: Verify uninstall
echo ========================================
powershell -Command "$devices = Get-PnpDevice | Where-Object { $_.InstanceId -like '*Idd*' }; if ($devices) { Write-Host 'FAILED: Devices still present:' -ForegroundColor Red; $devices | Format-Table FriendlyName, Status, InstanceId -AutoSize } else { Write-Host 'SUCCESS: All devices removed!' -ForegroundColor Green }"
echo.

echo ========================================
echo Test Complete
echo ========================================
pause
