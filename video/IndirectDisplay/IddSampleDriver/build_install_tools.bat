@echo off
echo ========================================
echo 編譯驅動程式安裝/卸載工具
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
    echo 警告: 找不到 Visual Studio，嘗試直接編譯...
)

echo.
echo 編譯 1/2: 帶回滾的安裝工具...
cl.exe /nologo /O2 /W3 /EHsc setupapi_install_with_rollback.cpp setupapi.lib newdev.lib cfgmgr32.lib /link /out:install_driver.exe

if %ERRORLEVEL% NEQ 0 (
    echo 編譯失敗
    pause
    exit /b 1
)

echo 成功: install_driver.exe
echo.

echo 編譯 2/2: 卸載工具...
cl.exe /nologo /O2 /W3 /EHsc setupapi_uninstall.cpp setupapi.lib newdev.lib cfgmgr32.lib /link /out:uninstall_driver.exe

if %ERRORLEVEL% NEQ 0 (
    echo 編譯失敗
    pause
    exit /b 1
)

echo 成功: uninstall_driver.exe
echo.

echo ========================================
echo 編譯完成
echo ========================================
echo.
echo 已創建以下工具:
echo   - install_driver.exe     (安裝驅動，帶回滾功能)
echo   - uninstall_driver.exe   (卸載驅動)
echo.
pause

