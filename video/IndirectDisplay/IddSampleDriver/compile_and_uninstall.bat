@echo off
echo ========================================
echo 編譯並卸載驅動程式 (使用 SetupAPI)
echo ========================================
echo.

REM 找到並設置 Visual Studio 環境
set "VS_PATH=C:\Program Files (x86)\Microsoft Visual Studio\2019"
if not exist "%VS_PATH%" set "VS_PATH=C:\Program Files\Microsoft Visual Studio\2022"
if not exist "%VS_PATH%" set "VS_PATH=C:\Program Files (x86)\Microsoft Visual Studio\2017"

if exist "%VS_PATH%\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" (
    call "%VS_PATH%\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x64
) else if exist "%VS_PATH%\Community\VC\Auxiliary\Build\vcvarsall.bat" (
    call "%VS_PATH%\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
) else if exist "%VS_PATH%\Professional\VC\Auxiliary\Build\vcvarsall.bat" (
    call "%VS_PATH%\Professional\VC\Auxiliary\Build\vcvarsall.bat" x64
) else (
    echo 錯誤: 找不到 Visual Studio
    echo.
    echo 嘗試手動編譯...
    goto :manual_compile
)

:compile
echo.
echo 編譯 setupapi_uninstall_simple.cpp...
cl.exe /nologo /O2 /W3 setupapi_uninstall_simple.cpp setupapi.lib /link /out:uninstall_driver.exe

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo 編譯失敗
    pause
    exit /b 1
)

echo.
echo 編譯成功!
echo.
goto :run

:manual_compile
REM 直接嘗試 cl.exe (如果已經在 PATH 中)
cl.exe /nologo /O2 /W3 setupapi_uninstall_simple.cpp setupapi.lib /link /out:uninstall_driver.exe 2>nul
if %ERRORLEVEL% EQU 0 goto :run

echo.
echo 無法編譯。請從 Visual Studio Developer Command Prompt 運行此批處理文件
pause
exit /b 1

:run
echo ========================================
echo 以管理員身份運行卸載程式...
echo ========================================
echo.

REM 以管理員身份運行
powershell -Command "Start-Process -FilePath '%CD%\uninstall_driver.exe' -Verb RunAs -Wait"

echo.
echo ========================================
echo 完成
echo ========================================
pause

