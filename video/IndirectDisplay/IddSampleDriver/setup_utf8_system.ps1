# 完整的 UTF-8 系統配置腳本
# 需要管理員權限

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "系統 UTF-8 配置工具" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# 檢查管理員權限
$isAdmin = ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)

if (-not $isAdmin) {
    Write-Host "錯誤: 需要管理員權限" -ForegroundColor Red
    Write-Host "請以管理員身份運行此腳本" -ForegroundColor Yellow
    exit 1
}

Write-Host "當前編碼狀態:" -ForegroundColor Yellow
Write-Host "----------------------------------------"
Write-Host "系統默認編碼: $([System.Text.Encoding]::Default.EncodingName)"
Write-Host "控制台輸出編碼: $([Console]::OutputEncoding.EncodingName)"
Write-Host "控制台輸入編碼: $([Console]::InputEncoding.EncodingName)"
Write-Host ""

# ============================================================================
# 1. 設置 PowerShell 為 UTF-8
# ============================================================================

Write-Host "[1] 配置 PowerShell UTF-8..." -ForegroundColor Cyan

# 當前會話
$OutputEncoding = [System.Text.Encoding]::UTF8
[Console]::OutputEncoding = [System.Text.Encoding]::UTF8
[Console]::InputEncoding = [System.Text.Encoding]::UTF8

# PowerShell 配置文件
$profilePath = $PROFILE.CurrentUserAllHosts
$profileDir = Split-Path $profilePath -Parent

if (!(Test-Path $profileDir)) {
    New-Item -ItemType Directory -Path $profileDir -Force | Out-Null
}

$utfConfig = @"
# UTF-8 Configuration
`$OutputEncoding = [System.Text.Encoding]::UTF8
[Console]::OutputEncoding = [System.Text.Encoding]::UTF8
[Console]::InputEncoding = [System.Text.Encoding]::UTF8
"@

if (Test-Path $profilePath) {
    $content = Get-Content $profilePath -Raw -ErrorAction SilentlyContinue
    if ($content -notlike "*OutputEncoding*") {
        Add-Content -Path $profilePath -Value "`n$utfConfig"
        Write-Host "  OK: 已添加到 PowerShell 配置文件" -ForegroundColor Green
    } else {
        Write-Host "  INFO: PowerShell 配置已存在" -ForegroundColor Yellow
    }
} else {
    Set-Content -Path $profilePath -Value $utfConfig -Encoding UTF8
    Write-Host "  OK: 已創建 PowerShell 配置文件" -ForegroundColor Green
}

# ============================================================================
# 2. 設置 CMD 為 UTF-8
# ============================================================================

Write-Host "`n[2] 配置 CMD UTF-8..." -ForegroundColor Cyan

# 設置註冊表讓 CMD 默認使用 UTF-8
$regPath = "HKCU:\Console"
Set-ItemProperty -Path $regPath -Name "CodePage" -Value 65001 -Type DWord -ErrorAction SilentlyContinue
Write-Host "  OK: CMD 代碼頁設置為 UTF-8 (65001)" -ForegroundColor Green

# ============================================================================
# 3. 設置系統區域為 UTF-8 (Beta: UTF-8 worldwide language support)
# ============================================================================

Write-Host "`n[3] 檢查 Beta UTF-8 支持..." -ForegroundColor Cyan

$regPath = "HKLM:\SYSTEM\CurrentControlSet\Control\Nls\CodePage"
$acp = Get-ItemProperty -Path $regPath -Name "ACP" -ErrorAction SilentlyContinue

if ($acp.ACP -eq "65001") {
    Write-Host "  OK: 系統已啟用 Beta UTF-8 支持" -ForegroundColor Green
} else {
    Write-Host "  INFO: 系統未啟用 Beta UTF-8 支持" -ForegroundColor Yellow
    Write-Host "  需要手動設置:" -ForegroundColor Yellow
    Write-Host "    1. 控制台 -> 時鐘和區域 -> 地區" -ForegroundColor White
    Write-Host "    2. 管理 -> 變更系統地區設定" -ForegroundColor White
    Write-Host "    3. 勾選「Beta: 使用 Unicode UTF-8 提供全球語言支援」" -ForegroundColor White
    Write-Host "    4. 重新啟動電腦" -ForegroundColor White
}

# ============================================================================
# 4. 設置 Visual Studio 編輯器為 UTF-8
# ============================================================================

Write-Host "`n[4] Visual Studio 設置提示..." -ForegroundColor Cyan
Write-Host "  請在 Visual Studio 中設置:" -ForegroundColor Yellow
Write-Host "    工具 -> 選項 -> 環境 -> 文件" -ForegroundColor White
Write-Host "    設置「預設編碼」為「UTF-8 (帶簽名) - 字碼頁 65001」" -ForegroundColor White

# ============================================================================
# 5. 設置 Git 為 UTF-8
# ============================================================================

Write-Host "`n[5] 配置 Git UTF-8..." -ForegroundColor Cyan

$gitPath = (Get-Command git -ErrorAction SilentlyContinue).Path

if ($gitPath) {
    & git config --global core.quotepath false
    & git config --global gui.encoding utf-8
    & git config --global i18n.commitencoding utf-8
    & git config --global i18n.logoutputencoding utf-8
    Write-Host "  OK: Git 已配置為 UTF-8" -ForegroundColor Green
} else {
    Write-Host "  INFO: Git 未安裝，跳過" -ForegroundColor Yellow
}

# ============================================================================
# 6. 創建開發環境啟動腳本
# ============================================================================

Write-Host "`n[6] 創建開發環境腳本..." -ForegroundColor Cyan

$devEnvScript = @'
@echo off
REM Development Environment with UTF-8
chcp 65001 > nul
set LANG=zh_TW.UTF-8
set LC_ALL=zh_TW.UTF-8

echo ========================================
echo Development Environment (UTF-8)
echo ========================================
echo.
echo Code Page: UTF-8 (65001)
echo.

cmd /k
'@

Set-Content -Path "dev_env_utf8.bat" -Value $devEnvScript -Encoding ASCII
Write-Host "  OK: 已創建 dev_env_utf8.bat" -ForegroundColor Green
Write-Host "  使用此批處理文件啟動 UTF-8 開發環境" -ForegroundColor White

# ============================================================================
# 7. 創建 Visual Studio 開發命令提示符 UTF-8 版本
# ============================================================================

Write-Host "`n[7] 創建 VS Developer Command Prompt (UTF-8)..." -ForegroundColor Cyan

$vsDevScript = @'
@echo off
REM Visual Studio Developer Command Prompt with UTF-8
chcp 65001 > nul

REM 查找並調用 vcvarsall.bat
set "VS_PATH=C:\Program Files\Microsoft Visual Studio\2022"
if not exist "%VS_PATH%" set "VS_PATH=C:\Program Files (x86)\Microsoft Visual Studio\2019"

if exist "%VS_PATH%\Community\VC\Auxiliary\Build\vcvarsall.bat" (
    call "%VS_PATH%\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
) else if exist "%VS_PATH%\Professional\VC\Auxiliary\Build\vcvarsall.bat" (
    call "%VS_PATH%\Professional\VC\Auxiliary\Build\vcvarsall.bat" x64
) else if exist "%VS_PATH%\Enterprise\VC\Auxiliary\Build\vcvarsall.bat" (
    call "%VS_PATH%\Enterprise\VC\Auxiliary\Build\vcvarsall.bat" x64
)

echo.
echo ========================================
echo VS Developer Command Prompt (UTF-8)
echo ========================================
echo.

cmd /k
'@

Set-Content -Path "vs_dev_utf8.bat" -Value $vsDevScript -Encoding ASCII
Write-Host "  OK: 已創建 vs_dev_utf8.bat" -ForegroundColor Green
Write-Host "  使用此批處理文件啟動 VS 開發環境" -ForegroundColor White

# ============================================================================
# 驗證配置
# ============================================================================

Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host "驗證當前編碼" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan

Write-Host ""
Write-Host "系統默認編碼: $([System.Text.Encoding]::Default.EncodingName)"
Write-Host "控制台輸出編碼: $([Console]::OutputEncoding.EncodingName)"
Write-Host "控制台輸入編碼: $([Console]::InputEncoding.EncodingName)"
Write-Host ""

# 測試中文顯示
Write-Host "中文測試: 驅動程式安裝工具" -ForegroundColor Green
Write-Host "English test: Driver Installation Tool" -ForegroundColor Green

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "配置完成" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "建議操作:" -ForegroundColor Yellow
Write-Host "1. 重新啟動 PowerShell/CMD 以應用設置" -ForegroundColor White
Write-Host "2. 在控制台設置「Beta UTF-8 支持」（需重啟電腦）" -ForegroundColor White
Write-Host "3. 重新編譯所有 C++ 源文件" -ForegroundColor White
Write-Host ""
Write-Host "快捷腳本:" -ForegroundColor Yellow
Write-Host "  dev_env_utf8.bat - UTF-8 開發環境" -ForegroundColor White
Write-Host "  vs_dev_utf8.bat  - VS 開發環境 (UTF-8)" -ForegroundColor White

