@echo off
REM 使用 Visual Studio 建置 VDD (不需要 CMake)
echo ========================================
echo VDD 建置腳本 (Visual Studio 版本)
echo ========================================

REM 設定路徑
set VDD_DIR=%~dp0
set VS_DIR=%VDD_DIR%IddSampleDriver

echo [INFO] VDD 目錄: %VDD_DIR%
echo [INFO] Visual Studio 專案目錄: %VS_DIR%

REM 檢查 Visual Studio 是否可用
echo [INFO] 檢查 Visual Studio...
where msbuild >nul 2>&1
if %errorLevel% neq 0 (
    echo [ERROR] 找不到 MSBuild，請安裝 Visual Studio
    echo [INFO] 請安裝 Visual Studio 2019 或更新版本
    pause
    exit /b 1
)

REM 步驟 1: 建置 VDD SDK 和 CLI 工具
echo.
echo ========================================
echo 步驟 1: 建置 VDD SDK 和 CLI 工具
echo ========================================

cd /d "%VS_DIR%"

REM 檢查是否有 CMakeLists.txt
if exist "CMakeLists.txt" (
    echo [INFO] 找到 CMakeLists.txt，嘗試使用 CMake...
    cmake --version >nul 2>&1
    if %errorLevel% == 0 (
        echo [INFO] 使用 CMake 建置...
        cmake -B build -S . -DBUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Release
        if %errorLevel% == 0 (
            cmake --build build --config Release
            if %errorLevel% == 0 (
                echo [SUCCESS] CMake 建置成功
                goto :copy_files
            )
        )
        echo [WARNING] CMake 建置失敗，嘗試其他方法...
    ) else (
        echo [WARNING] CMake 未安裝，跳過 CMake 建置...
    )
)

REM 嘗試使用 MSBuild 建置
echo [INFO] 使用 MSBuild 建置...
if exist "IddSampleDriver.sln" (
    msbuild IddSampleDriver.sln /p:Configuration=Release /p:Platform=x64 /p:TargetVersion=Windows10
    if %errorLevel% == 0 (
        echo [SUCCESS] MSBuild 建置成功
    ) else (
        echo [WARNING] MSBuild 建置可能失敗，繼續執行...
    )
) else (
    echo [WARNING] 找不到 IddSampleDriver.sln
)

:copy_files
REM 步驟 2: 複製檔案到輸出目錄
echo.
echo ========================================
echo 步驟 2: 複製檔案到輸出目錄
echo ========================================

REM 建立輸出目錄
if not exist "%VDD_DIR%x64\Debug" mkdir "%VDD_DIR%x64\Debug"
if not exist "%VDD_DIR%x64\Debug\IddSampleDriver" mkdir "%VDD_DIR%x64\Debug\IddSampleDriver"

REM 複製建置的檔案
if exist "build\Release\vddctl.exe" (
    copy "build\Release\vddctl.exe" "%VDD_DIR%x64\Debug\"
    echo [SUCCESS] vddctl.exe 複製完成
) else (
    echo [WARNING] 找不到 vddctl.exe，請檢查建置過程
)

if exist "build\Release\vddsdk.dll" (
    copy "build\Release\vddsdk.dll" "%VDD_DIR%x64\Debug\"
    echo [SUCCESS] vddsdk.dll 複製完成
)

if exist "build\Release\vddsdk.lib" (
    copy "build\Release\vddsdk.lib" "%VDD_DIR%x64\Debug\"
    echo [SUCCESS] vddsdk.lib 複製完成
)

REM 複製驅動程式檔案
if exist "x64\Debug\IddSampleDriver\IddSampleDriver.dll" (
    copy "x64\Debug\IddSampleDriver\IddSampleDriver.dll" "%VDD_DIR%x64\Debug\IddSampleDriver\"
    echo [SUCCESS] IddSampleDriver.dll 複製完成
)

if exist "x64\Debug\IddSampleDriver\IddSampleDriver.inf" (
    copy "x64\Debug\IddSampleDriver\IddSampleDriver.inf" "%VDD_DIR%x64\Debug\IddSampleDriver\"
    echo [SUCCESS] IddSampleDriver.inf 複製完成
)

if exist "x64\Debug\IddSampleDriver\iddsampledriver.cat" (
    copy "x64\Debug\IddSampleDriver\iddsampledriver.cat" "%VDD_DIR%x64\Debug\IddSampleDriver\"
    echo [SUCCESS] iddsampledriver.cat 複製完成
)

echo.
echo ========================================
echo 建置完成
echo ========================================
echo.
echo 建置的檔案:
echo - vddctl.exe     (CLI 工具)
echo - vddsdk.dll     (SDK 庫)
echo - vddsdk.lib     (SDK 靜態庫)
echo - IddSampleDriver.dll (驅動程式)
echo - IddSampleDriver.inf (驅動程式 INF)
echo.
echo 下一步: 執行 deploy_vdd.bat 部署 VDD
echo.

pause
