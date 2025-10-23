@echo off
REM 手動建置 vddctl.exe (不需要 CMake)
echo ========================================
echo 手動建置 vddctl.exe
echo ========================================

REM 設定路徑
set VDD_DIR=%~dp0
set VS_DIR=%VDD_DIR%IddSampleDriver

echo [INFO] VDD 目錄: %VDD_DIR%
echo [INFO] 專案目錄: %VS_DIR%

REM 檢查編譯器
echo [INFO] 檢查編譯器...
where cl >nul 2>&1
if %errorLevel% neq 0 (
    echo [ERROR] 找不到 Visual Studio 編譯器
    echo [INFO] 請開啟 "Developer Command Prompt for VS" 或 "x64 Native Tools Command Prompt"
    pause
    exit /b 1
)

REM 設定環境變數
echo [INFO] 設定編譯環境...
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
if %errorLevel% neq 0 (
    call "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
    if %errorLevel% neq 0 (
        echo [WARNING] 無法設定 Visual Studio 環境，嘗試繼續...
    )
)

REM 建立輸出目錄
if not exist "%VDD_DIR%x64\Debug" mkdir "%VDD_DIR%x64\Debug"

REM 編譯 vddctl.cpp
echo [INFO] 編譯 vddctl.cpp...
cd /d "%VS_DIR%"

REM 編譯命令
cl /EHsc /std:c++17 /I. vddctl.cpp vddsdk.cpp /Fe:"%VDD_DIR%x64\Debug\vddctl.exe" /link user32.lib kernel32.lib advapi32.lib setupapi.lib

if %errorLevel% == 0 (
    echo [SUCCESS] vddctl.exe 編譯成功
) else (
    echo [ERROR] vddctl.exe 編譯失敗
    echo [INFO] 請檢查編譯器設定和依賴項
    pause
    exit /b 1
)

REM 複製其他必要檔案
echo [INFO] 複製必要檔案...

REM 複製驅動程式檔案
if exist "x64\Debug\IddSampleDriver\IddSampleDriver.dll" (
    if not exist "%VDD_DIR%x64\Debug\IddSampleDriver" mkdir "%VDD_DIR%x64\Debug\IddSampleDriver"
    copy "x64\Debug\IddSampleDriver\IddSampleDriver.dll" "%VDD_DIR%x64\Debug\IddSampleDriver\"
    copy "x64\Debug\IddSampleDriver\IddSampleDriver.inf" "%VDD_DIR%x64\Debug\IddSampleDriver\"
    copy "x64\Debug\IddSampleDriver\iddsampledriver.cat" "%VDD_DIR%x64\Debug\IddSampleDriver\"
    echo [SUCCESS] 驅動程式檔案複製完成
) else (
    echo [WARNING] 找不到驅動程式檔案，請先建置驅動程式
)

echo.
echo ========================================
echo 手動建置完成
echo ========================================
echo.
echo 建置的檔案:
echo - vddctl.exe     (CLI 工具)
echo - IddSampleDriver.dll (驅動程式)
echo - IddSampleDriver.inf (驅動程式 INF)
echo.
echo 下一步: 執行 deploy_vdd.bat 部署 VDD
echo.

pause
