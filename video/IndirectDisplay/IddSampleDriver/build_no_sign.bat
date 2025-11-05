@echo off
REM ========================================
REM 编译驱动（跳过签名）
REM ========================================

echo.
echo ========================================
echo 编译 IddSampleDriver（跳过签名）
echo ========================================
echo.

cd /d "%~dp0"

echo 清理旧文件...
if exist "x64\Release\IddSampleDriver.dll" del /f /q "x64\Release\IddSampleDriver.dll"
if exist "x64\Release\IddSampleDriver.pdb" del /f /q "x64\Release\IddSampleDriver.pdb"
if exist "x64\Release\IddSampleDriver.inf" del /f /q "x64\Release\IddSampleDriver.inf"

echo.
echo 正在编译驱动...
"C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" ^
    IddSampleDriver.vcxproj ^
    /p:Configuration=Release ^
    /p:Platform=x64 ^
    /p:SignMode=Off ^
    /p:TestSign=false ^
    /p:Inf2CatBypass=true ^
    /p:RunCodeAnalysis=false ^
    /t:ClCompile,Link ^
    /v:minimal

if %errorLevel% EQU 0 (
    echo.
    echo ========================================
    echo [成功] 编译完成！
    echo ========================================
    echo.
    echo 输出文件：
    if exist "x64\Release\IddSampleDriver.dll" (
        echo   [✓] x64\Release\IddSampleDriver.dll
        dir "x64\Release\IddSampleDriver.dll" | findstr /C:"IddSampleDriver.dll"
    ) else (
        echo   [✗] x64\Release\IddSampleDriver.dll - 未找到！
    )
    
    echo.
    echo 复制 INF 文件...
    copy /Y "IddSampleDriver.inf" "x64\Release\IddSampleDriver.inf" >nul
    if exist "x64\Release\IddSampleDriver.inf" (
        echo   [✓] x64\Release\IddSampleDriver.inf
    )
    
    echo.
    echo ========================================
    echo 注意：驱动未签名，需要测试模式！
    echo ========================================
    echo.
    echo 下一步：
    echo 1. 启用测试模式：.\enable_test_mode.bat
    echo 2. 重启电脑
    echo 3. 安装驱动：.\build\bin\Release\vddctl.exe install x64\Release\IddSampleDriver.inf
    echo.
    
) else (
    echo.
    echo ========================================
    echo [错误] 编译失败！
    echo ========================================
    echo.
    echo 查看完整错误信息请重新运行此脚本
    echo.
)

pause




