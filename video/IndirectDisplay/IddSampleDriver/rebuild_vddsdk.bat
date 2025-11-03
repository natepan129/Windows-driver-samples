@echo off
cd /d "%~dp0"
echo Current directory: %CD%
echo.
echo Cleaning old DLL...
del /f /q build\bin\Release\vddsdk.dll 2>nul
del /f /q build\vddsdk.dir\Release\*.obj 2>nul
echo.
echo Rebuilding vddsdk...
"C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" build\vddsdk.vcxproj /p:Configuration=Release /p:Platform=x64 /t:Rebuild /v:minimal
echo.
echo Checking output...
dir build\bin\Release\vddsdk.* /o-d
echo.
pause

