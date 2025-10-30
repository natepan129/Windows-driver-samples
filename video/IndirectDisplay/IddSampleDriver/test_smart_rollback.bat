@echo off
echo ========================================
echo 编译智能回滚版本安装工具
echo ========================================

REM 清理旧文件
if exist setupapi_install_smart_rollback.exe del /F /Q setupapi_install_smart_rollback.exe

REM 编译
cl.exe /W4 /EHsc /O2 setupapi_install_smart_rollback.cpp /link /out:setupapi_install_smart_rollback.exe

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo 编译失败！错误码: %ERRORLEVEL%
    pause
    exit /b %ERRORLEVEL%
)

echo.
echo ========================================
echo 编译成功！开始安装...
echo ========================================
echo.

REM 以管理员权限运行
setupapi_install_smart_rollback.exe IddSampleDriver_Fixed.inf

echo.
echo ========================================
echo 安装完成，检查设备...
echo ========================================
echo.

powershell -Command "Get-PnpDevice | Where-Object { $_.InstanceId -like '*IddSampleDriver*' -or $_.InstanceId -like '*DISPLAY*' } | Select-Object -First 5 | Format-Table FriendlyName, Status, Class, InstanceId -AutoSize"

pause

