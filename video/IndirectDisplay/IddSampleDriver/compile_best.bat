@echo off
echo ========================================
echo 编译最佳版本（简单方法 + 回滚）
echo ========================================
echo.

REM 找到并设置 Visual Studio 环境
set "VS_PATH=C:\Program Files (x86)\Microsoft Visual Studio\2019"
if not exist "%VS_PATH%" set "VS_PATH=C:\Program Files\Microsoft Visual Studio\2022"

if exist "%VS_PATH%\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" (
    call "%VS_PATH%\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x64
) else if exist "%VS_PATH%\Community\VC\Auxiliary\Build\vcvarsall.bat" (
    call "%VS_PATH%\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
)

echo.
echo 编译 setupapi_install_best.cpp...
cl.exe /nologo /O2 /W3 /EHsc setupapi_install_best.cpp setupapi.lib newdev.lib cfgmgr32.lib /Fe:install_best.exe

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ========================================
    echo 编译成功: install_best.exe
    echo ========================================
    echo.
) else (
    echo.
    echo 编译失败
    pause
    exit /b 1
)

pause

