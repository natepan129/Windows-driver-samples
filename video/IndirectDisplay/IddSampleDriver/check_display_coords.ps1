# 檢查所有顯示器坐標
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "顯示器坐標檢查工具" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# 方法 1: 使用 .NET API
Write-Host "[方法 1] 使用 System.Windows.Forms.Screen" -ForegroundColor Yellow
Write-Host "----------------------------------------"
Add-Type -AssemblyName System.Windows.Forms
$screens = [System.Windows.Forms.Screen]::AllScreens
$index = 1
foreach ($screen in $screens) {
    $bounds = $screen.Bounds
    $isPrimary = if ($screen.Primary) { "⭐ 主顯示器" } else { "  次要顯示器" }
    
    Write-Host "顯示器 $index`: $isPrimary" -ForegroundColor Green
    Write-Host "  設備名稱: $($screen.DeviceName)"
    Write-Host "  位置: ($($bounds.X), $($bounds.Y))" -ForegroundColor Cyan
    Write-Host "  大小: $($bounds.Width) x $($bounds.Height)"
    Write-Host "  右邊界: $($bounds.Right)" -ForegroundColor Magenta
    Write-Host ""
    $index++
}

# 方法 2: 使用 WMI
Write-Host "[方法 2] 使用 WMI 查詢" -ForegroundColor Yellow
Write-Host "----------------------------------------"
$monitors = Get-CimInstance -Namespace root\wmi -ClassName WmiMonitorBasicDisplayParams -ErrorAction SilentlyContinue
if ($monitors) {
    $monitors | ForEach-Object {
        Write-Host "監視器: $($_.InstanceName)"
        Write-Host "  活躍: $($_.Active)"
    }
} else {
    Write-Host "  (VirtualBox 環境可能無法獲取 WMI 監視器信息)" -ForegroundColor Gray
}

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "預期的虛擬顯示器坐標：" -ForegroundColor Green
Write-Host "  VDD 1: (2560, 0)" -ForegroundColor Cyan
Write-Host "  VDD 2: (4480, 0)" -ForegroundColor Cyan
Write-Host "  VDD 3: (6400, 0)" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan

