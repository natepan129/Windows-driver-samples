@echo off
echo ========================================
echo 编译 VDD SDK
echo ========================================
echo.

REM 设置 Visual Studio 环境
set "VS_PATH=C:\Program Files (x86)\Microsoft Visual Studio\2019"
if not exist "%VS_PATH%" set "VS_PATH=C:\Program Files\Microsoft Visual Studio\2022"

if exist "%VS_PATH%\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" (
    call "%VS_PATH%\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x64
) else if exist "%VS_PATH%\Community\VC\Auxiliary\Build\vcvarsall.bat" (
    call "%VS_PATH%\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
) else (
    echo 错误: 找不到 Visual Studio
    pause
    exit /b 1
)

echo.
echo 使用 MSBuild 编译 vddsdk...
echo ----------------------------------------

cd build
msbuild VddSdk.sln /t:vddsdk /p:Configuration=Release /p:Platform=x64 /m

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ========================================
    echo 编译成功！
    echo ========================================
    echo.
    echo 输出位置:
    dir /b bin\Release\vddsdk.dll 2>nul
    dir /b lib\Release\vddsdk.lib 2>nul
    echo.
) else (
    echo.
    echo ========================================
    echo 编译失败！
    echo ========================================
)

cd ..
pause

