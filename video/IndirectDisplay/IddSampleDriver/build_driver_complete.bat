@echo off
REM ========================================
REM 完整编译驱动（生成未签名的 .cat）
REM ========================================

echo.
echo ========================================
echo 完整编译 IddSampleDriver
echo ========================================
echo.

cd /d "%~dp0"

REM ========================================
REM 步骤 1: 编译驱动 DLL
REM ========================================
echo [1/5] 编译驱动 DLL...
"C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" ^
    IddSampleDriver.vcxproj ^
    /p:Configuration=Release ^
    /p:Platform=x64 ^
    /t:ClCompile,Link ^
    /p:SignMode=Off ^
    /p:TestSign=false ^
    /v:minimal ^
    /nologo

if %errorLevel% NEQ 0 (
    echo [错误] 编译失败！
    pause
    exit /b 1
)

REM ========================================
REM 步骤 2: 复制并处理 INF 文件
REM ========================================
echo [2/5] 准备 INF 文件...

REM 使用 stampinf 添加版本信息
"C:\Program Files (x86)\Windows Kits\10\bin\10.0.26100.0\x86\stampinf.exe" ^
    -f "IddSampleDriver.inf" ^
    -d "*" ^
    -a "amd64" ^
    -v "*" ^
    -k "2.25.0"

REM 复制到输出目录
copy /Y "IddSampleDriver.inf" "x64\Release\" >nul

if %errorLevel% NEQ 0 (
    echo [警告] Stampinf 失败，直接复制 INF...
    copy /Y "IddSampleDriver.inf" "x64\Release\" >nul
)

REM ========================================
REM 步骤 3: 生成未签名的 .cat 文件
REM ========================================
echo [3/5] 生成 catalog 文件（不需要证书）...

REM 检查 inf2cat 工具
set INF2CAT="C:\Program Files (x86)\Windows Kits\10\bin\10.0.26100.0\x86\inf2cat.exe"
if not exist %INF2CAT% (
    echo [错误] 找不到 inf2cat.exe
    echo 路径: %INF2CAT%
    echo.
    echo 请确认 WDK 已正确安装
    pause
    exit /b 1
)

REM 生成 .cat 文件
%INF2CAT% /driver:x64\Release /os:10_X64,10_RS2_X64,10_RS3_X64,10_RS4_X64,10_RS5_X64,10_19H1_X64,10_VB_X64,10_CO_X64 /verbose

if %errorLevel% NEQ 0 (
    echo [错误] 生成 catalog 失败！
    pause
    exit /b 1
)

REM ========================================
REM 步骤 4: 验证输出文件
REM ========================================
echo [4/5] 验证输出文件...
echo.

set ALL_OK=1

if exist "x64\Release\IddSampleDriver.dll" (
    echo [✓] IddSampleDriver.dll
    for %%F in ("x64\Release\IddSampleDriver.dll") do echo     大小: %%~zF 字节
) else (
    echo [✗] IddSampleDriver.dll - 未找到！
    set ALL_OK=0
)

if exist "x64\Release\IddSampleDriver.inf" (
    echo [✓] IddSampleDriver.inf
) else (
    echo [✗] IddSampleDriver.inf - 未找到！
    set ALL_OK=0
)

if exist "x64\Release\IddSampleDriver.cat" (
    echo [✓] IddSampleDriver.cat （未签名）
    for %%F in ("x64\Release\IddSampleDriver.cat") do echo     大小: %%~zF 字节
) else (
    echo [✗] IddSampleDriver.cat - 未找到！
    set ALL_OK=0
)

echo.

if %ALL_OK%==0 (
    echo [错误] 部分文件未生成！
    pause
    exit /b 1
)

REM ========================================
REM 步骤 5: 显示后续步骤
REM ========================================
echo [5/5] 完成！
echo.
echo ========================================
echo 编译成功！文件列表：
echo ========================================
dir /B "x64\Release\IddSampleDriver.*"
echo.
echo ========================================
echo 后续步骤：
echo ========================================
echo.
echo 选项 A：测试签名（开发测试）
echo   1. 启用测试模式：.\enable_test_mode.bat
echo   2. 重启电脑
echo   3. 安装驱动：.\build\bin\Release\vddctl.exe install x64\Release\IddSampleDriver.inf
echo.
echo 选项 B：商业签名（正式发布）
echo   1. 购买 EV 代码签名证书
echo   2. 签名 .cat 文件：
echo      signtool sign /sha1 [证书指纹] x64\Release\IddSampleDriver.cat
echo   3. 提交到 Microsoft Hardware Dev Center
echo   4. 等待审核（3-7 天）
echo   5. 下载 Microsoft 签名的驱动
echo.
echo ========================================
echo 当前状态：已生成未签名的驱动包
echo ========================================
echo.

pause




