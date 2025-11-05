@echo off
REM ========================================
REM 简单编译驱动（跳过签名和验证）
REM ========================================

echo.
echo ========================================
echo 编译 IddSampleDriver（简化版）
echo ========================================
echo.

cd /d "%~dp0"

REM 清理旧文件
echo [1/4] 清理旧文件...
if exist "x64\Release\Driver.obj" del /f /q "x64\Release\Driver.obj" >nul 2>&1
if exist "x64\Release\IddSampleDriver.dll" del /f /q "x64\Release\IddSampleDriver.dll" >nul 2>&1

REM 编译源代码
echo [2/4] 编译源代码...
"C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" ^
    IddSampleDriver.vcxproj ^
    /p:Configuration=Release ^
    /p:Platform=x64 ^
    /t:ClCompile ^
    /v:minimal ^
    /nologo

if %errorLevel% NEQ 0 (
    echo [错误] 编译失败！
    pause
    exit /b 1
)

REM 链接生成 DLL
echo [3/4] 链接 DLL...
"C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" ^
    IddSampleDriver.vcxproj ^
    /p:Configuration=Release ^
    /p:Platform=x64 ^
    /t:Link ^
    /v:minimal ^
    /nologo

if %errorLevel% NEQ 0 (
    echo [错误] 链接失败！
    pause
    exit /b 1
)

REM 复制 INF 文件
echo [4/4] 复制 INF 文件...
copy /Y "IddSampleDriver.inf" "x64\Release\" >nul

REM 检查结果
echo.
echo ========================================
echo 编译完成！
echo ========================================
echo.

if exist "x64\Release\IddSampleDriver.dll" (
    echo [✓] DLL 文件已生成
    dir "x64\Release\IddSampleDriver.dll" | findstr /C:"IddSampleDriver.dll"
) else (
    echo [✗] DLL 文件未找到！
    pause
    exit /b 1
)

if exist "x64\Release\IddSampleDriver.inf" (
    echo [✓] INF 文件已复制
) else (
    echo [✗] INF 文件未找到！
)

echo.
echo ========================================
echo 注意：驱动未签名！
echo ========================================
echo.
echo 安装前必须：
echo 1. 启用测试模式：.\enable_test_mode.bat
echo 2. 重启电脑
echo 3. 安装驱动：.\build\bin\Release\vddctl.exe install x64\Release\IddSampleDriver.inf
echo.

pause




