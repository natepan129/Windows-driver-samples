@echo off
REM 構建 UMDF 驅動程序 DLL

echo ========================================
echo 構建 UMDF 驅動程序 DLL
echo ========================================

REM 檢查管理員權限
net session >nul 2>&1
if %errorlevel% neq 0 (
    echo ❌ 需要管理員權限
    echo 請右鍵點擊此文件，選擇"以管理員身份運行"
    pause
    exit /b 1
)

echo ✅ 管理員權限確認

echo.
echo ========================================
echo 步驟 1: 檢查 Visual Studio 環境
echo ========================================
echo 檢查 MSBuild...
where msbuild
if %errorlevel% neq 0 (
    echo ❌ MSBuild 未找到
    echo 請安裝 Visual Studio 或 Build Tools
    pause
    exit /b 1
)

echo ✅ MSBuild 找到

echo.
echo ========================================
echo 步驟 2: 構建 UMDF 驅動程序
echo ========================================
echo 構建 IddSampleDriver.dll...
msbuild IddSampleDriver.vcxproj /p:Configuration=Release /p:Platform=x64 /p:TargetVersion=Windows10

if %errorlevel% neq 0 (
    echo ❌ 構建失敗
    echo 請檢查錯誤信息
    pause
    exit /b 1
)

echo ✅ 構建成功

echo.
echo ========================================
echo 步驟 3: 檢查構建結果
echo ========================================
echo 檢查 DLL 文件...
dir "x64\Release\IddSampleDriver.dll" 2>nul
if %errorlevel% neq 0 (
    echo ❌ DLL 文件未找到
    echo 檢查構建輸出目錄...
    dir x64\Release\
    pause
    exit /b 1
)

echo ✅ DLL 文件找到

echo.
echo ========================================
echo 步驟 4: 複製文件到正確位置
echo ========================================
echo 創建 UMDF 目錄...
if not exist "UMDF" mkdir UMDF

echo 複製 DLL 文件...
copy "x64\Release\IddSampleDriver.dll" "UMDF\"
if %errorlevel% neq 0 (
    echo ❌ 複製失敗
    pause
    exit /b 1
)

echo ✅ 文件複製成功

echo.
echo ========================================
echo 步驟 5: 檢查最終文件
echo ========================================
echo 檢查 INF 文件...
dir IddSampleDriver.inf

echo 檢查 DLL 文件...
dir UMDF\IddSampleDriver.dll

echo.
echo ========================================
echo 步驟 6: 嘗試安裝驅動程序
echo ========================================
echo 安裝 UMDF 驅動程序...
pnputil /add-driver "IddSampleDriver.inf" /install

echo.
echo ========================================
echo 步驟 7: 檢查安裝結果
echo ========================================
echo 檢查 ROOT 設備：
pnputil /enum-devices | findstr -i "root.*idd"

echo.
echo 檢查所有 IddSampleDriver 設備：
pnputil /enum-devices | findstr -i "idd"

echo.
echo ========================================
echo 完成
echo ========================================
pause


