@echo off
REM 解決 IddSampleDriver 數位簽名問題的完整方案

echo ========================================
echo 解決 IddSampleDriver 數位簽名問題
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
echo 步驟 1: 檢查測試模式狀態
echo ========================================
echo 檢查測試簽名狀態...
bcdedit /enum | findstr -i "testsigning"

echo.
echo ========================================
echo 步驟 2: 強制啟用測試模式
echo ========================================
echo 啟用測試簽名模式...
bcdedit /set testsigning on

echo 啟用開發者模式...
bcdedit /set bootdebug on

echo 禁用驅動程序簽名強制...
bcdedit /set nointegritychecks on

echo.
echo ========================================
echo 步驟 3: 使用不同的安裝方法
echo ========================================
echo 方法 1: 使用 pnputil 強制安裝...
pnputil /add-driver "IddSampleDriver.inf" /install /force

echo.
echo 方法 2: 使用 devcon 安裝...
if exist "C:\Program Files (x86)\Windows Kits\10\Tools\x64\devcon.exe" (
    "C:\Program Files (x86)\Windows Kits\10\Tools\x64\devcon.exe" install "IddSampleDriver.inf" "Root\IddSampleDriver"
) else (
    echo devcon.exe 未找到，跳過此方法
)

echo.
echo 方法 3: 使用 SetupAPI 直接安裝...
powershell -Command "& {Add-Type -AssemblyName System.Windows.Forms; [System.Windows.Forms.MessageBox]::Show('請手動執行 SetupAPI 安裝', '提示')}"

echo.
echo ========================================
echo 步驟 4: 檢查安裝結果
echo ========================================
echo 檢查 ROOT 設備：
pnputil /enum-devices | findstr -i "root.*idd"

echo.
echo 檢查所有 IddSampleDriver 設備：
pnputil /enum-devices | findstr -i "idd"

echo.
echo ========================================
echo 步驟 5: 如果仍然失敗，嘗試替代方案
echo ========================================
echo 創建自簽名證書...
if not exist "IddSampleDriver.cer" (
    echo 生成自簽名證書...
    makecert -r -pe -ss PrivateCertStore -n "CN=IddSampleDriver" IddSampleDriver.cer
)

echo 簽名 INF 文件...
if exist "IddSampleDriver.cer" (
    signtool sign /f IddSampleDriver.cer /p "" IddSampleDriver.inf
)

echo 重新嘗試安裝...
pnputil /add-driver "IddSampleDriver.inf" /install

echo.
echo ========================================
echo 步驟 6: 測試 SDK
echo ========================================
echo 測試 VDD SDK...
build\bin\Release\vddctl.exe init
build\bin\Release\vddctl.exe status

echo.
echo ========================================
echo 重要提醒
echo ========================================
echo ⚠️  如果仍然失敗，可能需要：
echo 1. 重新啟動系統以完全啟用測試模式
echo 2. 使用 Visual Studio 編譯並簽名驅動程序
echo 3. 使用 Windows Driver Kit (WDK) 工具
echo.
echo ========================================
echo 完成
echo ========================================
pause



