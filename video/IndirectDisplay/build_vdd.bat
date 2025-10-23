@echo off
REM VDD 建置腳本
REM 建置 VDD SDK、CLI 工具和驅動程式

echo ========================================
echo VDD 建置腳本
echo ========================================

REM 設定路徑
set VDD_DIR=%~dp0
set BUILD_DIR=%VDD_DIR%IddSampleDriver\build

echo [INFO] VDD 目錄: %VDD_DIR%
echo [INFO] 建置目錄: %BUILD_DIR%

REM 步驟 1: 建置 VDD SDK 和 CLI 工具
echo.
echo ========================================
echo 步驟 1: 建置 VDD SDK 和 CLI 工具
echo ========================================

cd /d "%VDD_DIR%IddSampleDriver"

REM 使用 CMake 建置
echo [INFO] 配置 CMake...
cmake -B build -S . -DBUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Release
if %errorLevel% neq 0 (
    echo [ERROR] CMake 配置失敗
    pause
    exit /b 1
)

echo [INFO] 建置專案...
cmake --build build --config Release
if %errorLevel% neq 0 (
    echo [ERROR] CMake 建置失敗
    pause
    exit /b 1
)

REM 步驟 2: 建置驅動程式
echo.
echo ========================================
echo 步驟 2: 建置驅動程式
echo ========================================

echo [INFO] 建置驅動程式...
msbuild IddSampleDriver.sln /p:Configuration=Release /p:Platform=x64 /p:TargetVersion=Windows10
if %errorLevel% neq 0 (
    echo [WARNING] 驅動程式建置可能失敗，繼續執行...
)

REM 步驟 3: 複製檔案到輸出目錄
echo.
echo ========================================
echo 步驟 3: 複製檔案到輸出目錄
echo ========================================

REM 建立輸出目錄
if not exist "%VDD_DIR%x64\Debug" mkdir "%VDD_DIR%x64\Debug"

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
    echo [SUCCESS] 驅動程式檔案已存在
) else (
    echo [WARNING] 驅動程式檔案不存在，請檢查建置過程
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
