@echo off
echo ========================================
echo VDD Control Tool - Init Command Test
echo ========================================
echo.

set VDDCTL=build\bin\Release\vddctl.exe

if not exist "%VDDCTL%" (
    echo ERROR: vddctl.exe not found
    echo Please build it first: cmd /c compile_sdk_only.bat
    pause
    exit /b 1
)

echo ========================================
echo Example 1: Basic Init
echo ========================================
echo Command: vddctl init
echo.
"%VDDCTL%" init
echo.
pause

echo ========================================
echo Example 2: Init with Verbose Logging
echo ========================================
echo Command: vddctl init --verbose
echo.
"%VDDCTL%" init --verbose
echo.
pause

echo ========================================
echo Example 3: Check Status After Init
echo ========================================
echo Command: vddctl status
echo.
"%VDDCTL%" status
echo.
pause

echo ========================================
echo Init Command Summary
echo ========================================
echo.
echo vddctl init 的作用：
echo   [1] 初始化 VDD SDK
echo   [2] 加载必要的配置
echo   [3] 准备驱动管理环境
echo   [4] 检查系统兼容性
echo.
echo 选项：
echo   --verbose  显示详细日志
echo.
echo 返回值：
echo   Success: SDK initialized successfully
echo   Already: SDK already initialized  
echo   Error: 显示具体错误信息
echo.
pause

