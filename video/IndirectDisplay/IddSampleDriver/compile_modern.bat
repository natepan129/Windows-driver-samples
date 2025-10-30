@echo off
echo Compiling modern install method...

REM Find VS environment
set "VS_PATH=C:\Program Files (x86)\Microsoft Visual Studio\2019"
if not exist "%VS_PATH%" set "VS_PATH=C:\Program Files\Microsoft Visual Studio\2022"

if exist "%VS_PATH%\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" (
    call "%VS_PATH%\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x64
) else if exist "%VS_PATH%\Community\VC\Auxiliary\Build\vcvarsall.bat" (
    call "%VS_PATH%\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
)

cl.exe /nologo /O2 /W3 /EHsc setupapi_install_modern.cpp setupapi.lib newdev.lib /Fe:install_modern.exe

if %ERRORLEVEL% EQU 0 (
    echo SUCCESS: install_modern.exe
) else (
    echo FAILED
)

pause

