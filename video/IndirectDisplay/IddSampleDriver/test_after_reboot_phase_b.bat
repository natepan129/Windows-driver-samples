@echo off
REM ==============================================================================
REM 重启后 Phase B ContainerId 验证
REM ==============================================================================

echo.
echo ========================================
echo Phase B ContainerId 稳定性验证
echo 重启后测试
echo ========================================
echo.

echo [步驟 1] 检查驱动状态
echo ----------------------------------------
.\build\bin\Release\vddctl.exe status
echo.

echo [步驟 2] 列出显示器
echo ----------------------------------------
.\build\bin\Release\vddctl.exe list
echo.

echo 预期结果：
echo - Display Active: Yes  ← 应该仍然激活
echo - Active Displays: 3   ← 3 个虚拟显示器
echo.

echo ========================================
echo 关键测试：显示器布局和鼠标对齐
echo ========================================
echo.
echo 请检查以下内容（不要运行 activate）：
echo.
echo 1. 显示器布局是否与重启前相同？
echo    - 在 Windows 显示设置中查看
echo    - 虚拟显示器位置应该保持不变
echo.
echo 2. 鼠标对齐是否准确？
echo    - 点击桌面不同位置
echo    - 拖动窗口
echo    - 点击按钮
echo.
pause

echo.
echo ========================================
echo 请回答以下问题：
echo ========================================
echo.
set /p LAYOUT="显示器布局是否保持不变？(Y/N): "
set /p MOUSE="鼠标对齐是否准确？(Y/N): "
set /p ACTIVE="虚拟显示器是否仍然激活？(Y/N): "

echo.
echo ========================================
echo 测试结果总结
echo ========================================
echo.
echo 显示器布局保持: %LAYOUT%
echo 鼠标对齐准确:   %MOUSE%
echo 虚拟显示器激活: %ACTIVE%
echo.

if /i "%LAYOUT%"=="Y" if /i "%MOUSE%"=="Y" if /i "%ACTIVE%"=="Y" (
    echo ========================================
    echo ✅✅✅ Phase B 测试通过！✅✅✅
    echo ========================================
    echo.
    echo ContainerId 稳定性修复成功验证：
    echo - ✅ 显示器布局在重启后保持不变
    echo - ✅ 鼠标坐标系统稳定
    echo - ✅ Windows 正确识别设备为"已知设备"
    echo.
    echo 🎉 Phase A+B 所有修复完全成功！
    echo.
) else (
    echo ========================================
    echo ⚠️ Phase B 测试部分失败
    echo ========================================
    echo.
    if /i not "%LAYOUT%"=="Y" (
        echo ❌ 显示器布局重置了
        echo    → ContainerId 可能需要进一步调试
    )
    if /i not "%MOUSE%"=="Y" (
        echo ❌ 鼠标对齐不准确
        echo    → 可能需要重新 activate
    )
    if /i not "%ACTIVE%"=="Y" (
        echo ❌ 虚拟显示器未激活
        echo    → 这是正常的，重启后需要重新 activate
        echo    → 但 ContainerId 修复应该保持布局
    )
    echo.
    echo 建议：运行 activate 并重新测试
    echo .\build\bin\Release\vddctl.exe activate
)

echo.
pause


