# VDD Simple Install Script
# Fixed encoding and syntax issues

param(
    [switch]$Install,
    [switch]$Uninstall,
    [switch]$Status,
    [switch]$Test,
    [switch]$Help
)

# Set paths
$VDD_DIR = "C:\Users\WDKRemoteUser\source\repos\IndirectDisplay"
$DRIVER_DIR = "$VDD_DIR\x64\Debug\IddSampleDriver"

# Check administrator privileges
function Test-Administrator {
    $currentUser = [Security.Principal.WindowsIdentity]::GetCurrent()
    $principal = New-Object Security.Principal.WindowsPrincipal($currentUser)
    return $principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
}

# Install VDD driver
function Install-VDD {
    Write-Host "=== Installing VDD Driver ===" -ForegroundColor Green
    
    if (-not (Test-Administrator)) {
        Write-Host "Error: Administrator privileges required!" -ForegroundColor Red
        return $false
    }
    
    if (-not (Test-Path "$DRIVER_DIR\IddSampleDriver.dll")) {
        Write-Host "Error: Driver DLL not found!" -ForegroundColor Red
        return $false
    }
    
    if (-not (Test-Path "$DRIVER_DIR\IddSampleDriver.inf")) {
        Write-Host "Error: Driver INF not found!" -ForegroundColor Red
        return $false
    }
    
    Write-Host "Found driver files, installing..." -ForegroundColor Yellow
    
    try {
        $result = pnputil /add-driver "$DRIVER_DIR\IddSampleDriver.inf" /install
        if ($LASTEXITCODE -eq 0) {
            Write-Host "Driver installed successfully!" -ForegroundColor Green
            return $true
        } else {
            Write-Host "Driver installation failed!" -ForegroundColor Red
            return $false
        }
    } catch {
        Write-Host "Error during installation: $($_.Exception.Message)" -ForegroundColor Red
        return $false
    }
}

# Uninstall VDD driver
function Uninstall-VDD {
    Write-Host "=== Uninstalling VDD Driver ===" -ForegroundColor Green
    
    if (-not (Test-Administrator)) {
        Write-Host "Error: Administrator privileges required!" -ForegroundColor Red
        return $false
    }
    
    try {
        $result = pnputil /delete-driver "IddSampleDriver.inf" /uninstall
        if ($LASTEXITCODE -eq 0) {
            Write-Host "Driver uninstalled successfully!" -ForegroundColor Green
            return $true
        } else {
            Write-Host "Driver uninstallation failed!" -ForegroundColor Red
            return $false
        }
    } catch {
        Write-Host "Error during uninstallation: $($_.Exception.Message)" -ForegroundColor Red
        return $false
    }
}

# Check VDD status
function Get-VDDStatus {
    Write-Host "=== VDD Status Check ===" -ForegroundColor Green
    
    # Check if driver is installed
    $drivers = pnputil /enum-drivers | Select-String "IddSampleDriver"
    if ($drivers) {
        Write-Host "Driver is installed" -ForegroundColor Green
        $drivers | ForEach-Object { Write-Host "  $_" -ForegroundColor Cyan }
    } else {
        Write-Host "Driver is not installed" -ForegroundColor Red
    }
    
    # Check devices
    $devices = Get-PnpDevice | Where-Object {$_.FriendlyName -like "*Idd*"}
    if ($devices) {
        Write-Host "Found VDD devices:" -ForegroundColor Green
        $devices | ForEach-Object { Write-Host "  $($_.FriendlyName) - $($_.Status)" -ForegroundColor Cyan }
    } else {
        Write-Host "No VDD devices found" -ForegroundColor Red
    }
    
    # Check monitors
    $monitors = Get-WmiObject -Class Win32_DesktopMonitor
    Write-Host "Monitor information:" -ForegroundColor Yellow
    $monitors | ForEach-Object { Write-Host "  $($_.Name) - $($_.Status)" -ForegroundColor Cyan }
    
    # Check virtual monitors
    $virtualMonitors = Get-WmiObject -Class Win32_DesktopMonitor | Where-Object {$_.Name -like "*Virtual*"}
    if ($virtualMonitors) {
        Write-Host "Found virtual monitors:" -ForegroundColor Green
        $virtualMonitors | ForEach-Object { Write-Host "  $($_.Name)" -ForegroundColor Cyan }
    } else {
        Write-Host "No virtual monitors found" -ForegroundColor Red
    }
}

# Test VDD functionality
function Test-VDD {
    Write-Host "=== Testing VDD Functionality ===" -ForegroundColor Green
    
    if (Test-Path "$VDD_DIR\x64\Debug\IddSampleApp.exe") {
        Write-Host "Found sample application, starting test..." -ForegroundColor Yellow
        
        try {
            Start-Process "$VDD_DIR\x64\Debug\IddSampleApp.exe" -PassThru
            Write-Host "Sample application started" -ForegroundColor Green
            
            Start-Sleep -Seconds 3
            
            $monitors = Get-WmiObject -Class Win32_DesktopMonitor
            Write-Host "Current monitor count: $($monitors.Count)" -ForegroundColor Yellow
            
        } catch {
            Write-Host "Failed to start sample application: $($_.Exception.Message)" -ForegroundColor Red
        }
    } else {
        Write-Host "Sample application not found!" -ForegroundColor Red
    }
}

# Show help
function Show-Help {
    Write-Host "=== VDD VirtualBox Deployment Tool ===" -ForegroundColor Green
    Write-Host ""
    Write-Host "Usage:" -ForegroundColor Yellow
    Write-Host "  .\vdd_simple_install.ps1 -Install    # Install VDD" -ForegroundColor Cyan
    Write-Host "  .\vdd_simple_install.ps1 -Uninstall  # Uninstall VDD" -ForegroundColor Cyan
    Write-Host "  .\vdd_simple_install.ps1 -Status     # Check status" -ForegroundColor Cyan
    Write-Host "  .\vdd_simple_install.ps1 -Test       # Test functionality" -ForegroundColor Cyan
    Write-Host "  .\vdd_simple_install.ps1 -Help        # Show help" -ForegroundColor Cyan
    Write-Host ""
    Write-Host "Examples:" -ForegroundColor Yellow
    Write-Host "  .\vdd_simple_install.ps1 -Install" -ForegroundColor Cyan
    Write-Host "  .\vdd_simple_install.ps1 -Status" -ForegroundColor Cyan
    Write-Host "  .\vdd_simple_install.ps1 -Test" -ForegroundColor Cyan
    Write-Host ""
    Write-Host "Note: Administrator privileges required" -ForegroundColor Red
}

# Main logic
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
