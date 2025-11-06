@echo off
REM ==============================================================================
REM 诊断虚拟显示器坐标
REM ==============================================================================

echo.
echo ========================================
echo 虚拟显示器坐标诊断
echo ========================================
echo.

echo [1] 查询当前显示器坐标（Windows API）
echo ----------------------------------------
.\build\bin\Release\vddctl.exe list
echo.

echo [2] 查询注册表中的坐标配置
echo ----------------------------------------
echo 正在查询注册表...
echo.

reg query "HKLM\SYSTEM\CurrentControlSet\Control\GraphicsDrivers\Configuration" /s | findstr /i "position scale"

echo.
echo [3] 查询虚拟显示器的 DeviceKey
echo ----------------------------------------
reg query "HKLM\SYSTEM\CurrentControlSet\Enum\ROOT\IddSampleDriver" /s /f "Attach.ToDesktop" | findstr /i "Attach Position"

echo.
echo ========================================
echo 诊断完成
echo ========================================
pause


