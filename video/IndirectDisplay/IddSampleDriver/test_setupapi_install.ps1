# 簡化的 SetupAPI 安裝測試腳本

# 檢查管理員權限
$isAdmin = ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "SetupAPI 驅動程式安裝測試" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

if (-not $isAdmin) {
    Write-Host "⚠ 警告: 未以管理員身份運行" -ForegroundColor Yellow
    Write-Host "某些操作可能會失敗" -ForegroundColor Yellow
} else {
    Write-Host "✓ 以管理員身份運行" -ForegroundColor Green
}

Write-Host ""

# 檢查 INF 文件
$infPath = "IddSampleDriver_Fixed.inf"

if (-not (Test-Path $infPath)) {
    Write-Host "✗ 找不到 INF 文件: $infPath" -ForegroundColor Red
    exit 1
}

$infPath = (Resolve-Path $infPath).Path
Write-Host "INF 文件: $infPath" -ForegroundColor Green
Write-Host ""

# ============================================================================
# 方法 1: pnputil (最簡單且最可靠)
# ============================================================================

Write-Host "【方法 1】使用 pnputil" -ForegroundColor Yellow
Write-Host "----------------------------------------" -ForegroundColor Gray

Write-Host "執行: pnputil /add-driver `"$infPath`" /install" -ForegroundColor Cyan

try {
    $output = & pnputil.exe /add-driver "$infPath" /install 2>&1 | Out-String
    
    Write-Host $output
    
    if ($LASTEXITCODE -eq 0) {
        Write-Host "✓ pnputil 執行成功 (返回碼: $LASTEXITCODE)" -ForegroundColor Green
    } else {
        Write-Host "✗ pnputil 執行失敗 (返回碼: $LASTEXITCODE)" -ForegroundColor Red
    }
} catch {
    Write-Host "✗ pnputil 執行出錯: $_" -ForegroundColor Red
}

Write-Host ""

# ============================================================================
# 方法 2: 使用 devcon (如果可用)
# ============================================================================

Write-Host "【方法 2】使用 devcon (如果可用)" -ForegroundColor Yellow
Write-Host "----------------------------------------" -ForegroundColor Gray

$devconPaths = @(
    "C:\Program Files (x86)\Windows Kits\10\Tools\x64\devcon.exe",
    "C:\Program Files (x86)\Windows Kits\10\Tools\ARM64\devcon.exe",
    ".\devcon.exe"
)

$devconPath = $null
foreach ($path in $devconPaths) {
    if (Test-Path $path) {
        $devconPath = $path
        break
    }
}

if ($devconPath) {
    Write-Host "找到 devcon: $devconPath" -ForegroundColor Green
    
    Write-Host "執行: devcon install `"$infPath`" ROOT\IddSampleDriver" -ForegroundColor Cyan
    
    try {
        $output = & $devconPath install "$infPath" "ROOT\IddSampleDriver" 2>&1 | Out-String
        Write-Host $output
        
        if ($LASTEXITCODE -eq 0) {
            Write-Host "✓ devcon 執行成功" -ForegroundColor Green
        } else {
            Write-Host "✗ devcon 執行失敗 (返回碼: $LASTEXITCODE)" -ForegroundColor Red
        }
    } catch {
        Write-Host "✗ devcon 執行出錯: $_" -ForegroundColor Red
    }
} else {
    Write-Host "⚠ 找不到 devcon.exe，跳過此方法" -ForegroundColor Yellow
}

Write-Host ""

# ============================================================================
# 驗證安裝結果
# ============================================================================

Write-Host "【驗證】檢查安裝結果" -ForegroundColor Yellow
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# 1. 檢查驅動程式存儲區
Write-Host "1. 檢查驅動程式存儲區:" -ForegroundColor Cyan
$driverList = & pnputil.exe /enum-drivers 2>&1 | Out-String

if ($driverList -match "iddsampledriver") {
    Write-Host "   ✓ 驅動程式已在驅動程式存儲區中" -ForegroundColor Green
    
    # 提取 OEM 編號
    if ($driverList -match "發行的名稱\s*:\s*(oem\d+\.inf)") {
        $oemName = $matches[1]
        Write-Host "   發行的名稱: $oemName" -ForegroundColor White
    }
    if ($driverList -match "Published Name\s*:\s*(oem\d+\.inf)") {
        $oemName = $matches[1]
        Write-Host "   Published Name: $oemName" -ForegroundColor White
    }
} else {
    Write-Host "   ✗ 驅動程式不在驅動程式存儲區中" -ForegroundColor Red
}

Write-Host ""

# 2. 檢查註冊表
Write-Host "2. 檢查註冊表:" -ForegroundColor Cyan

$registryChecks = @{
    "WUDFRd 服務" = "HKLM:\SYSTEM\CurrentControlSet\Services\WUDFRd"
    "IddSampleDriver 設備" = "HKLM:\SYSTEM\CurrentControlSet\Enum\ROOT\IddSampleDriver"
    "IndirectKmd 服務" = "HKLM:\SYSTEM\CurrentControlSet\Services\IndirectKmd"
}

foreach ($item in $registryChecks.GetEnumerator()) {
    if (Test-Path $item.Value) {
        Write-Host "   ✓ $($item.Key): 已找到" -ForegroundColor Green
    } else {
        Write-Host "   ✗ $($item.Key): 未找到" -ForegroundColor Yellow
    }
}

Write-Host ""

# 3. 檢查設備管理器
Write-Host "3. 檢查設備管理器:" -ForegroundColor Cyan

try {
    # 檢查所有顯示適配器
    $displayDevices = Get-PnpDevice -Class Display 2>$null
    
    if ($displayDevices) {
        Write-Host "   找到的顯示設備:" -ForegroundColor White
        foreach ($device in $displayDevices) {
            $symbol = if ($device.Status -eq "OK") { "✓" } else { "✗" }
            $color = if ($device.Status -eq "OK") { "Green" } else { "Yellow" }
            
            Write-Host "   $symbol $($device.FriendlyName) - 狀態: $($device.Status)" -ForegroundColor $color
        }
    }
    
    # 專門檢查我們的驅動程式
    $iddDevices = Get-PnpDevice | Where-Object { 
        $_.FriendlyName -like "*IddSampleDriver*" -or 
        $_.InstanceId -like "*IddSampleDriver*" 
    }
    
    if ($iddDevices) {
        Write-Host "`n   ✓ 找到 IddSampleDriver 設備:" -ForegroundColor Green
        foreach ($device in $iddDevices) {
            Write-Host "     - $($device.FriendlyName)" -ForegroundColor White
            Write-Host "       ID: $($device.InstanceId)" -ForegroundColor Gray
            Write-Host "       狀態: $($device.Status)" -ForegroundColor Gray
        }
    } else {
        Write-Host "   ⚠ 未找到 IddSampleDriver 設備" -ForegroundColor Yellow
    }
} catch {
    Write-Host "   ✗ 無法查詢設備: $_" -ForegroundColor Red
}

Write-Host ""

# 4. 檢查驅動程式文件
Write-Host "4. 檢查驅動程式文件:" -ForegroundColor Cyan

$driverPaths = @(
    "$env:SystemRoot\System32\drivers\IddSampleDriver.sys",
    "$env:SystemRoot\System32\IddSampleDriver.dll",
    "$env:SystemRoot\System32\DriverStore\FileRepository\iddsampledriver*"
)

foreach ($path in $driverPaths) {
    if (Test-Path $path) {
        Write-Host "   ✓ 找到: $path" -ForegroundColor Green
        
        # 如果是通配符路徑，顯示找到的所有項目
        if ($path -like "*`**") {
            $items = Get-ChildItem $path -Directory 2>$null
            foreach ($item in $items) {
                Write-Host "     → $($item.FullName)" -ForegroundColor Gray
            }
        }
    } else {
        Write-Host "   ✗ 未找到: $path" -ForegroundColor Yellow
    }
}

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "測試完成" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan


