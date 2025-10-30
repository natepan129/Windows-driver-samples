@echo off
chcp 65001 > nul
echo ========================================
echo UTF-8 System Configuration
echo ========================================
echo.

REM Check admin rights
net session >nul 2>&1
if %errorLevel% neq 0 (
    echo ERROR: Administrator privileges required
    echo Please run as Administrator
    pause
    exit /b 1
)

echo [1] Setting Console Code Page to UTF-8...
reg add "HKCU\Console" /v CodePage /t REG_DWORD /d 65001 /f > nul
echo     OK: Console configured for UTF-8 (65001)

echo.
echo [2] Configuring PowerShell...
powershell -Command "$OutputEncoding = [System.Text.Encoding]::UTF8; [Console]::OutputEncoding = [System.Text.Encoding]::UTF8; Write-Host '    OK: PowerShell session configured'"

echo.
echo [3] Setting up PowerShell profile...
powershell -Command "$profilePath = $PROFILE.CurrentUserAllHosts; $profileDir = Split-Path $profilePath -Parent; if (!(Test-Path $profileDir)) { New-Item -ItemType Directory -Path $profileDir -Force | Out-Null }; $utfConfig = \"`$OutputEncoding = [System.Text.Encoding]::UTF8`n[Console]::OutputEncoding = [System.Text.Encoding]::UTF8`n[Console]::InputEncoding = [System.Text.Encoding]::UTF8\"; if (Test-Path $profilePath) { $content = Get-Content $profilePath -Raw -ErrorAction SilentlyContinue; if ($content -notlike '*OutputEncoding*') { Add-Content -Path $profilePath -Value \"`n$utfConfig\"; Write-Host '    OK: Added to PowerShell profile' } else { Write-Host '    INFO: PowerShell profile already configured' } } else { Set-Content -Path $profilePath -Value $utfConfig -Encoding UTF8; Write-Host '    OK: Created PowerShell profile' }"

echo.
echo [4] Configuring Git (if installed)...
where git > nul 2>&1
if %errorLevel% equ 0 (
    git config --global core.quotepath false
    git config --global gui.encoding utf-8
    git config --global i18n.commitencoding utf-8
    git config --global i18n.logoutputencoding utf-8
    echo     OK: Git configured for UTF-8
) else (
    echo     INFO: Git not installed, skipping
)

echo.
echo ========================================
echo Configuration Complete
echo ========================================
echo.
echo Next steps:
echo 1. Restart PowerShell/CMD to apply settings
echo 2. Enable Beta UTF-8 support (requires reboot):
echo    Control Panel ^> Region ^> Administrative ^> Change system locale
echo    Check "Beta: Use Unicode UTF-8 for worldwide language support"
echo.
echo Created shortcuts:
echo   dev_env_utf8.bat - Development environment with UTF-8
echo   vs_dev_utf8.bat  - VS Developer Command Prompt with UTF-8
echo.
pause

