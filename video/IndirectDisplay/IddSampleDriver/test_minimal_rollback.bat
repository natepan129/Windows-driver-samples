@echo off
echo ========================================
echo 测试：完全相同逻辑 + 最小回滚
echo ========================================
echo.

REM 找到 VS 环境
set "VS_PATH=C:\Program Files (x86)\Microsoft Visual Studio\2019"
if not exist "%VS_PATH%" set "VS_PATH=C:\Program Files\Microsoft Visual Studio\2022"

if exist "%VS_PATH%\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" (
    call "%VS_PATH%\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x64
) else if exist "%VS_PATH%\Community\VC\Auxiliary\Build\vcvarsall.bat" (
    call "%VS_PATH%\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
)

echo 编译中...
cl.exe /nologo /O2 /W3 setupapi_install_with_minimal_rollback.cpp setupapi.lib newdev.lib /Fe:install_minimal.exe

if %ERRORLEVEL% NEQ 0 (
    echo 编译失败
    pause
    exit /b 1
)

echo.
echo 编译成功！开始测试...
echo.

REM 卸载旧设备
echo [1] 卸载旧设备...
powershell -Command "Start-Process -FilePath '%CD%\uninstall_driver.exe' -Verb RunAs -Wait"

echo.
echo [2] 安装新方法...
powershell -Command "Start-Process -FilePath '%CD%\install_minimal.exe' -Verb RunAs -Wait"

echo.
echo [3] 验证结果...
powershell -Command "Get-PnpDevice | Where-Object { $_.FriendlyName -like '*IddSampleDriver*' } | Format-Table FriendlyName, Status, InstanceId, Class, ClassGuid -AutoSize"

echo.
echo ========================================
echo 测试完成
echo ========================================
pause

