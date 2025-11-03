@echo off
cd /d "%~dp0"
"C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" build\VddSdk.sln /p:Configuration=Release /p:Platform=x64 /v:minimal
if errorlevel 1 (
    echo Build failed!
    pause
    exit /b 1
)

echo.
echo Copying files from WDKRemoteUser to current directory...
xcopy /y "C:\Users\WDKRemoteUser\Desktop\Windows-driver-samples\video\IndirectDisplay\IddSampleDriver\build\bin\Release\*" build\bin\Release\
xcopy /y "C:\Users\WDKRemoteUser\Desktop\Windows-driver-samples\video\IndirectDisplay\IddSampleDriver\build\lib\Release\*" build\lib\Release\

echo.
echo Files in current build directory:
dir build\bin\Release\vdd*.*
echo.
pause

