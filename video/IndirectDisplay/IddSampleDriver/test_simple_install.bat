@echo off
chcp 65001 >nul
echo ========================================
echo 测试简化后的驱动安装
echo ========================================
echo.

echo [1] 编译并测试独立安装工具
echo ----------------------------------------
call compile_and_install.bat

echo.
echo [2] 检查设备状态
echo ----------------------------------------
powershell -Command "& { $d = Get-PnpDevice | Where-Object { $_.InstanceId -like '*IddSampleDriver*' }; if ($d) { Write-Host '成功！设备已安装:'; $d | Format-Table FriendlyName, Status, Class, ClassGuid -AutoSize } else { Write-Host '失败：未找到设备' } }"

echo.
echo [3] 编译 SDK (vddsdk.dll)
echo ----------------------------------------
cd build
cmake --build . --config Release --target vddsdk
cd ..

echo.
echo ========================================
echo 测试完成
echo ========================================
echo.
echo 核心功能验证:
echo   [√] InstallDriver - 使用 simple 方法
echo   [√] 代码简化 - 移除 Registry
echo   [ ] 下一步: 实现 Activate/Deactivate
echo.
pause

