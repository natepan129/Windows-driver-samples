# SetupAPI Driver Installation Test Script (English)

# Check Administrator privileges
$isAdmin = ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "SetupAPI Driver Installation Test" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

if (-not $isAdmin) {
    Write-Host "WARNING: Not running as Administrator" -ForegroundColor Yellow
    Write-Host "Some operations may fail" -ForegroundColor Yellow
} else {
    Write-Host "OK: Running as Administrator" -ForegroundColor Green
}

Write-Host ""

# Check INF file
$infPath = "IddSampleDriver_Fixed.inf"

if (-not (Test-Path $infPath)) {
    Write-Host "ERROR: INF file not found: $infPath" -ForegroundColor Red
    exit 1
}

$infPath = (Resolve-Path $infPath).Path
Write-Host "INF file: $infPath" -ForegroundColor Green
Write-Host ""

# ============================================================================
# Method 1: pnputil (simplest and most reliable)
# ============================================================================

Write-Host "[Method 1] Using pnputil" -ForegroundColor Yellow
Write-Host "----------------------------------------" -ForegroundColor Gray

Write-Host "Executing: pnputil /add-driver `"$infPath`" /install" -ForegroundColor Cyan

try {
    $output = & pnputil.exe /add-driver "$infPath" /install 2>&1 | Out-String
    
    Write-Host $output
    
    if ($LASTEXITCODE -eq 0) {
        Write-Host "SUCCESS: pnputil completed (exit code: $LASTEXITCODE)" -ForegroundColor Green
    } else {
        Write-Host "FAILED: pnputil failed (exit code: $LASTEXITCODE)" -ForegroundColor Red
    }
} catch {
    Write-Host "ERROR: pnputil execution error: $_" -ForegroundColor Red
}

Write-Host ""

# ============================================================================
# Method 2: devcon (if available)
# ============================================================================

Write-Host "[Method 2] Using devcon (if available)" -ForegroundColor Yellow
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
    Write-Host "Found devcon: $devconPath" -ForegroundColor Green
    
    Write-Host "Executing: devcon install `"$infPath`" ROOT\IddSampleDriver" -ForegroundColor Cyan
    
    try {
        $output = & $devconPath install "$infPath" "ROOT\IddSampleDriver" 2>&1 | Out-String
        Write-Host $output
        
        if ($LASTEXITCODE -eq 0) {
            Write-Host "SUCCESS: devcon completed" -ForegroundColor Green
        } else {
            Write-Host "FAILED: devcon failed (exit code: $LASTEXITCODE)" -ForegroundColor Red
        }
    } catch {
        Write-Host "ERROR: devcon execution error: $_" -ForegroundColor Red
    }
} else {
    Write-Host "WARNING: devcon.exe not found, skipping this method" -ForegroundColor Yellow
}

Write-Host ""

# ============================================================================
# Verify installation results
# ============================================================================

Write-Host "[Verification] Checking installation results" -ForegroundColor Yellow
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# 1. Check driver store
Write-Host "1. Check Driver Store:" -ForegroundColor Cyan
$driverList = & pnputil.exe /enum-drivers 2>&1 | Out-String

if ($driverList -match "iddsampledriver") {
    Write-Host "   OK: Driver found in driver store" -ForegroundColor Green
    
    # Extract OEM name
    if ($driverList -match "Published Name\s*:\s*(oem\d+\.inf)") {
        $oemName = $matches[1]
        Write-Host "   Published Name: $oemName" -ForegroundColor White
    }
} else {
    Write-Host "   FAILED: Driver not in driver store" -ForegroundColor Red
}

Write-Host ""

# 2. Check registry
Write-Host "2. Check Registry:" -ForegroundColor Cyan

$registryChecks = @{
    "WUDFRd Service" = "HKLM:\SYSTEM\CurrentControlSet\Services\WUDFRd"
    "IddSampleDriver Device" = "HKLM:\SYSTEM\CurrentControlSet\Enum\ROOT\IddSampleDriver"
    "IndirectKmd Service" = "HKLM:\SYSTEM\CurrentControlSet\Services\IndirectKmd"
}

foreach ($item in $registryChecks.GetEnumerator()) {
    if (Test-Path $item.Value) {
        Write-Host "   OK: $($item.Key) found" -ForegroundColor Green
    } else {
        Write-Host "   NOT FOUND: $($item.Key)" -ForegroundColor Yellow
    }
}

Write-Host ""

# 3. Check Device Manager
Write-Host "3. Check Device Manager:" -ForegroundColor Cyan

try {
    # Check all display adapters
    $displayDevices = Get-PnpDevice -Class Display 2>$null
    
    if ($displayDevices) {
        Write-Host "   Display devices found:" -ForegroundColor White
        foreach ($device in $displayDevices) {
            $symbol = if ($device.Status -eq "OK") { "OK" } else { "!!" }
            $color = if ($device.Status -eq "OK") { "Green" } else { "Yellow" }
            
            Write-Host "   [$symbol] $($device.FriendlyName) - Status: $($device.Status)" -ForegroundColor $color
        }
    }
    
    # Check specifically for our driver
    $iddDevices = Get-PnpDevice | Where-Object { 
        $_.FriendlyName -like "*IddSampleDriver*" -or 
        $_.InstanceId -like "*IddSampleDriver*" 
    }
    
    if ($iddDevices) {
        Write-Host "`n   OK: IddSampleDriver device found:" -ForegroundColor Green
        foreach ($device in $iddDevices) {
            Write-Host "     - $($device.FriendlyName)" -ForegroundColor White
            Write-Host "       ID: $($device.InstanceId)" -ForegroundColor Gray
            Write-Host "       Status: $($device.Status)" -ForegroundColor Gray
        }
    } else {
        Write-Host "   WARNING: IddSampleDriver device not found" -ForegroundColor Yellow
    }
} catch {
    Write-Host "   ERROR: Cannot query devices: $_" -ForegroundColor Red
}

Write-Host ""

# 4. Check driver files
Write-Host "4. Check Driver Files:" -ForegroundColor Cyan

$driverPaths = @(
    "$env:SystemRoot\System32\drivers\IddSampleDriver.sys",
    "$env:SystemRoot\System32\IddSampleDriver.dll",
    "$env:SystemRoot\System32\DriverStore\FileRepository\iddsampledriver*"
)

foreach ($path in $driverPaths) {
    if (Test-Path $path) {
        Write-Host "   OK: Found $path" -ForegroundColor Green
        
        # If wildcard path, show all found items
        if ($path -like "*`**") {
            $items = Get-ChildItem $path -Directory 2>$null
            foreach ($item in $items) {
                Write-Host "     -> $($item.FullName)" -ForegroundColor Gray
            }
        }
    } else {
        Write-Host "   NOT FOUND: $path" -ForegroundColor Yellow
    }
}

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Test Complete" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan


