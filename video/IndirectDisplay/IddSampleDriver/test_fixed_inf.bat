@echo off
chcp 65001 >nul
echo ========================================
echo 测试修复后的 INF 文件
echo ========================================
echo.

echo [步骤 1] 卸载旧设备
echo ----------------------------------------
powershell -Command "& { $d = Get-PnpDevice | Where-Object { $_.InstanceId -like '*IddSampleDriver*' }; if ($d) { foreach ($dev in $d) { Write-Host '正在移除:' $dev.InstanceId; pnputil /remove-device $dev.InstanceId } } else { Write-Host '没有找到旧设备' } }"

echo.
echo [步骤 2] 使用修复后的 INF 重新安装
echo ----------------------------------------
pause

REM 重新编译安装工具
echo 编译安装工具...
call compile_and_install.bat

echo.
echo ========================================
echo 安装完成！
echo ========================================
pause
