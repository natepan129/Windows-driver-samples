# VDD VirtualBox 完整部署 PowerShell 腳本
# 在 VirtualBox 中直接使用命令列部署 VDD

param(
    [switch]$Install,
    [switch]$Uninstall,
    [switch]$Status,
    [switch]$Test,
    [switch]$Help
)

# 設定路徑
$VDD_DIR = "C:\Users\WDKRemoteUser\source\repos\IndirectDisplay"
$DRIVER_DIR = "$VDD_DIR\x64\Debug\IddSampleDriver"

# 顏色函數
function Write-ColorOutput($ForegroundColor) {
    $fc = $host.UI.RawUI.ForegroundColor
    $host.UI.RawUI.ForegroundColor = $ForegroundColor
    if ($args) {
        Write-Output $args
    } else {
        $input | Write-Output
    }
    $host.UI.RawUI.ForegroundColor = $fc
}

# 檢查管理員權限
function Test-Administrator {
    $currentUser = [Security.Principal.WindowsIdentity]::GetCurrent()
    $principal = New-Object Security.Principal.WindowsPrincipal($currentUser)
    return $principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
}

# 安裝 VDD 驅動程式
function Install-VDD {
    Write-ColorOutput "=== 安裝 VDD 驅動程式 ===" Green
    
    # 檢查管理員權限
    if (-not (Test-Administrator)) {
        Write-ColorOutput "錯誤: 需要管理員權限！" Red
        return $false
    }
    
    # 檢查檔案
    if (-not (Test-Path "$DRIVER_DIR\IddSampleDriver.dll")) {
        Write-ColorOutput "錯誤: 找不到驅動程式 DLL！" Red
        return $false
    }
    
    if (-not (Test-Path "$DRIVER_DIR\IddSampleDriver.inf")) {
        Write-ColorOutput "錯誤: 找不到驅動程式 INF！" Red
        return $false
    }
    
    Write-ColorOutput "找到驅動程式檔案，開始安裝..." Yellow
    
    # 安裝驅動程式
    try {
        $result = pnputil /add-driver "$DRIVER_DIR\IddSampleDriver.inf" /install
        if ($LASTEXITCODE -eq 0) {
            Write-ColorOutput "驅動程式安裝成功！" Green
            return $true
        } else {
            Write-ColorOutput "驅動程式安裝失敗！" Red
            return $false
        }
    } catch {
        Write-ColorOutput "安裝過程中發生錯誤: $($_.Exception.Message)" Red
        return $false
    }
}

# 卸載 VDD 驅動程式
function Uninstall-VDD {
    Write-ColorOutput "=== 卸載 VDD 驅動程式 ===" Green
    
    if (-not (Test-Administrator)) {
        Write-ColorOutput "錯誤: 需要管理員權限！" Red
        return $false
    }
    
    try {
        $result = pnputil /delete-driver "IddSampleDriver.inf" /uninstall
        if ($LASTEXITCODE -eq 0) {
            Write-ColorOutput "驅動程式卸載成功！" Green
            return $true
        } else {
            Write-ColorOutput "驅動程式卸載失敗！" Red
            return $false
        }
    } catch {
        Write-ColorOutput "卸載過程中發生錯誤: $($_.Exception.Message)" Red
        return $false
    }
}

# 檢查 VDD 狀態
function Get-VDDStatus {
    Write-ColorOutput "=== VDD 狀態檢查 ===" Green
    
    # 檢查驅動程式是否已安裝
    $drivers = pnputil /enum-drivers | Select-String "IddSampleDriver"
    if ($drivers) {
        Write-ColorOutput "✓ 驅動程式已安裝" Green
        $drivers | ForEach-Object { Write-ColorOutput "  $_" Cyan }
    } else {
        Write-ColorOutput "✗ 驅動程式未安裝" Red
    }
    
    # 檢查裝置管理員
    $devices = Get-PnpDevice | Where-Object {$_.FriendlyName -like "*Idd*"}
    if ($devices) {
        Write-ColorOutput "✓ 找到 VDD 裝置:" Green
        $devices | ForEach-Object { Write-ColorOutput "  $($_.FriendlyName) - $($_.Status)" Cyan }
    } else {
        Write-ColorOutput "✗ 未找到 VDD 裝置" Red
    }
    
    # 檢查顯示器
    $monitors = Get-WmiObject -Class Win32_DesktopMonitor
    Write-ColorOutput "顯示器資訊:" Yellow
    $monitors | ForEach-Object { Write-ColorOutput "  $($_.Name) - $($_.Status)" Cyan }
    
    # 檢查虛擬顯示器
    $virtualMonitors = Get-WmiObject -Class Win32_DesktopMonitor | Where-Object {$_.Name -like "*Virtual*"}
    if ($virtualMonitors) {
        Write-ColorOutput "✓ 找到虛擬顯示器:" Green
        $virtualMonitors | ForEach-Object { Write-ColorOutput "  $($_.Name)" Cyan }
    } else {
        Write-ColorOutput "✗ 未找到虛擬顯示器" Red
    }
}

# 測試 VDD 功能
function Test-VDD {
    Write-ColorOutput "=== 測試 VDD 功能 ===" Green
    
    # 檢查範例應用程式
    if (Test-Path "$VDD_DIR\x64\Debug\IddSampleApp.exe") {
        Write-ColorOutput "找到範例應用程式，開始測試..." Yellow
        
        try {
            Start-Process "$VDD_DIR\x64\Debug\IddSampleApp.exe" -PassThru
            Write-ColorOutput "範例應用程式已啟動" Green
            
            # 等待一下讓應用程式啟動
            Start-Sleep -Seconds 3
            
            # 檢查是否有新的虛擬顯示器
            $monitors = Get-WmiObject -Class Win32_DesktopMonitor
            Write-ColorOutput "當前顯示器數量: $($monitors.Count)" Yellow
            
        } catch {
            Write-ColorOutput "啟動範例應用程式失敗: $($_.Exception.Message)" Red
        }
    } else {
        Write-ColorOutput "找不到範例應用程式！" Red
    }
}

# 顯示幫助
function Show-Help {
    Write-ColorOutput "=== VDD VirtualBox 部署工具 ===" Green
    Write-ColorOutput ""
    Write-ColorOutput "使用方法:" Yellow
    Write-ColorOutput "  .\vdd_virtualbox_commands.ps1 -Install    # 安裝 VDD" Cyan
    Write-ColorOutput "  .\vdd_virtualbox_commands.ps1 -Uninstall  # 卸載 VDD" Cyan
    Write-ColorOutput "  .\vdd_virtualbox_commands.ps1 -Status     # 檢查狀態" Cyan
    Write-ColorOutput "  .\vdd_virtualbox_commands.ps1 -Test       # 測試功能" Cyan
    Write-ColorOutput "  .\vdd_virtualbox_commands.ps1 -Help        # 顯示幫助" Cyan
    Write-ColorOutput ""
    Write-ColorOutput "範例:" Yellow
    Write-ColorOutput "  .\vdd_virtualbox_commands.ps1 -Install" Cyan
    Write-ColorOutput "  .\vdd_virtualbox_commands.ps1 -Status" Cyan
    Write-ColorOutput "  .\vdd_virtualbox_commands.ps1 -Test" Cyan
    Write-ColorOutput ""
    Write-ColorOutput "注意: 需要管理員權限執行" Red
}

# 主程式邏輯
if ($Help) {
    Show-Help
} elseif ($Install) {
    Install-VDD
} elseif ($Uninstall) {
    Uninstall-VDD
} elseif ($Status) {
    Get-VDDStatus
} elseif ($Test) {
    Test-VDD
} else {
    Show-Help
}
