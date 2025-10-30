# Pure SetupAPI Driver Installation Script
# Using Windows SetupAPI functions directly via P/Invoke

param(
    [string]$InfPath = "IddSampleDriver_Fixed.inf"
)

# Check Administrator
$isAdmin = ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "SetupAPI Driver Installation" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

if (-not $isAdmin) {
    Write-Host "ERROR: Administrator privileges required" -ForegroundColor Red
    exit 1
}

Write-Host "OK: Running as Administrator" -ForegroundColor Green

# Check INF file
if (-not (Test-Path $InfPath)) {
    Write-Host "ERROR: INF file not found: $InfPath" -ForegroundColor Red
    exit 1
}

$InfPath = (Resolve-Path $InfPath).Path
Write-Host "INF file: $InfPath" -ForegroundColor Green
Write-Host ""

# ============================================================================
# Define SetupAPI P/Invoke declarations
# ============================================================================

Write-Host "Loading SetupAPI definitions..." -ForegroundColor Cyan

Add-Type -TypeDefinition @"
using System;
using System.Runtime.InteropServices;
using System.Text;

public class SetupAPI {
    // GUID for Display adapter class
    public static Guid GUID_DEVCLASS_DISPLAY = new Guid("{4D36E968-E325-11CE-BFC1-08002BE10318}");
    
    // Constants
    public const uint DICD_GENERATE_ID = 0x00000001;
    public const uint DIGCF_PRESENT = 0x00000002;
    public const uint DIGCF_ALLCLASSES = 0x00000004;
    public const uint DIGCF_PROFILE = 0x00000008;
    
    public const uint SPDRP_HARDWAREID = 0x00000001;
    public const uint SPDRP_COMPATIBLEIDS = 0x00000002;
    
    public const uint DIF_REGISTERDEVICE = 0x00000019;
    public const uint DIF_INSTALLDEVICE = 0x00000002;
    public const uint DIF_REMOVE = 0x00000005;
    
    public const uint INSTALLFLAG_FORCE = 0x00000001;
    public const uint INSTALLFLAG_NONINTERACTIVE = 0x00000004;
    
    public const uint SPINT_ACTIVE = 0x00000001;
    
    // Structures
    [StructLayout(LayoutKind.Sequential)]
    public struct SP_DEVINFO_DATA {
        public uint cbSize;
        public Guid ClassGuid;
        public uint DevInst;
        public IntPtr Reserved;
    }
    
    [StructLayout(LayoutKind.Sequential)]
    public struct SP_CLASSINSTALL_HEADER {
        public uint cbSize;
        public uint InstallFunction;
    }
    
    // SetupAPI Functions
    [DllImport("setupapi.dll", CharSet = CharSet.Unicode, SetLastError = true)]
    public static extern IntPtr SetupDiCreateDeviceInfoList(
        ref Guid ClassGuid,
        IntPtr hwndParent
    );
    
    [DllImport("setupapi.dll", CharSet = CharSet.Unicode, SetLastError = true)]
    public static extern bool SetupDiCreateDeviceInfo(
        IntPtr DeviceInfoSet,
        string DeviceName,
        ref Guid ClassGuid,
        string DeviceDescription,
        IntPtr hwndParent,
        uint CreationFlags,
        ref SP_DEVINFO_DATA DeviceInfoData
    );
    
    [DllImport("setupapi.dll", CharSet = CharSet.Unicode, SetLastError = true)]
    public static extern bool SetupDiSetDeviceRegistryProperty(
        IntPtr DeviceInfoSet,
        ref SP_DEVINFO_DATA DeviceInfoData,
        uint Property,
        byte[] PropertyBuffer,
        uint PropertyBufferSize
    );
    
    [DllImport("setupapi.dll", CharSet = CharSet.Unicode, SetLastError = true)]
    public static extern bool SetupDiCallClassInstaller(
        uint InstallFunction,
        IntPtr DeviceInfoSet,
        ref SP_DEVINFO_DATA DeviceInfoData
    );
    
    [DllImport("setupapi.dll", CharSet = CharSet.Unicode, SetLastError = true)]
    public static extern bool SetupDiDestroyDeviceInfoList(
        IntPtr DeviceInfoSet
    );
    
    [DllImport("setupapi.dll", CharSet = CharSet.Unicode, SetLastError = true)]
    public static extern IntPtr SetupDiGetClassDevs(
        ref Guid ClassGuid,
        string Enumerator,
        IntPtr hwndParent,
        uint Flags
    );
    
    [DllImport("setupapi.dll", CharSet = CharSet.Unicode, SetLastError = true)]
    public static extern bool SetupDiEnumDeviceInfo(
        IntPtr DeviceInfoSet,
        uint MemberIndex,
        ref SP_DEVINFO_DATA DeviceInfoData
    );
    
    [DllImport("setupapi.dll", CharSet = CharSet.Unicode, SetLastError = true)]
    public static extern IntPtr SetupOpenInfFile(
        string FileName,
        string InfClass,
        uint InfStyle,
        out uint ErrorLine
    );
    
    [DllImport("setupapi.dll", CharSet = CharSet.Unicode, SetLastError = true)]
    public static extern void SetupCloseInfFile(
        IntPtr InfHandle
    );
    
    [DllImport("setupapi.dll", CharSet = CharSet.Unicode, SetLastError = true)]
    public static extern bool SetupDiSetSelectedDevice(
        IntPtr DeviceInfoSet,
        ref SP_DEVINFO_DATA DeviceInfoData
    );
    
    // newdev.dll functions
    [DllImport("newdev.dll", CharSet = CharSet.Unicode, SetLastError = true)]
    public static extern bool UpdateDriverForPlugAndPlayDevices(
        IntPtr hwndParent,
        string HardwareId,
        string FullInfPath,
        uint InstallFlags,
        out bool bRebootRequired
    );
    
    [DllImport("newdev.dll", CharSet = CharSet.Unicode, SetLastError = true)]
    public static extern bool DiInstallDriver(
        IntPtr hwndParent,
        string FullInfPath,
        uint Flags,
        out bool NeedReboot
    );
    
    [DllImport("kernel32.dll", SetLastError = true)]
    public static extern uint GetLastError();
}
"@

Write-Host "OK: SetupAPI loaded" -ForegroundColor Green
Write-Host ""

# ============================================================================
# Method 1: UpdateDriverForPlugAndPlayDevices
# ============================================================================

Write-Host "[Method 1] UpdateDriverForPlugAndPlayDevices" -ForegroundColor Yellow
Write-Host "----------------------------------------" -ForegroundColor Gray

$hardwareId = "ROOT\IddSampleDriver"
$flags = [SetupAPI]::INSTALLFLAG_FORCE -bor [SetupAPI]::INSTALLFLAG_NONINTERACTIVE
$rebootRequired = $false

Write-Host "Hardware ID: $hardwareId" -ForegroundColor White
Write-Host "INF Path: $InfPath" -ForegroundColor White
Write-Host "Flags: 0x$($flags.ToString('X'))" -ForegroundColor White
Write-Host ""

try {
    $result = [SetupAPI]::UpdateDriverForPlugAndPlayDevices(
        [IntPtr]::Zero,
        $hardwareId,
        $InfPath,
        $flags,
        [ref]$rebootRequired
    )
    
    if ($result) {
        Write-Host "SUCCESS: Driver installed via UpdateDriverForPlugAndPlayDevices" -ForegroundColor Green
        if ($rebootRequired) {
            Write-Host "WARNING: Reboot required" -ForegroundColor Yellow
        }
    } else {
        $errorCode = [System.Runtime.InteropServices.Marshal]::GetLastWin32Error()
        Write-Host "FAILED: Error code $errorCode (0x$($errorCode.ToString('X')))" -ForegroundColor Red
        
        # Common error codes
        switch ($errorCode) {
            2 { Write-Host "  ERROR_FILE_NOT_FOUND" -ForegroundColor Red }
            5 { Write-Host "  ERROR_ACCESS_DENIED" -ForegroundColor Red }
            87 { Write-Host "  ERROR_INVALID_PARAMETER" -ForegroundColor Red }
            259 { Write-Host "  ERROR_NO_MORE_ITEMS (device not found)" -ForegroundColor Red }
            1168 { Write-Host "  ERROR_NOT_FOUND" -ForegroundColor Red }
            3758096385 { Write-Host "  E_INVALIDARG" -ForegroundColor Red }
            default { Write-Host "  Unknown error" -ForegroundColor Red }
        }
    }
} catch {
    Write-Host "EXCEPTION: $_" -ForegroundColor Red
}

Write-Host ""

# ============================================================================
# Method 2: DiInstallDriver
# ============================================================================

Write-Host "[Method 2] DiInstallDriver" -ForegroundColor Yellow
Write-Host "----------------------------------------" -ForegroundColor Gray

$needReboot = $false

Write-Host "INF Path: $InfPath" -ForegroundColor White
Write-Host ""

try {
    $result = [SetupAPI]::DiInstallDriver(
        [IntPtr]::Zero,
        $InfPath,
        $flags,
        [ref]$needReboot
    )
    
    if ($result) {
        Write-Host "SUCCESS: Driver installed via DiInstallDriver" -ForegroundColor Green
        if ($needReboot) {
            Write-Host "WARNING: Reboot required" -ForegroundColor Yellow
        }
    } else {
        $errorCode = [System.Runtime.InteropServices.Marshal]::GetLastWin32Error()
        Write-Host "FAILED: Error code $errorCode (0x$($errorCode.ToString('X')))" -ForegroundColor Red
    }
} catch {
    Write-Host "EXCEPTION: $_" -ForegroundColor Red
}

Write-Host ""

# ============================================================================
# Method 3: Manual device creation with SetupDiCreateDeviceInfo
# ============================================================================

Write-Host "[Method 3] SetupDiCreateDeviceInfo (Manual)" -ForegroundColor Yellow
Write-Host "----------------------------------------" -ForegroundColor Gray

try {
    $classGuid = [SetupAPI]::GUID_DEVCLASS_DISPLAY
    
    Write-Host "Step 1: Create device info list..." -ForegroundColor Cyan
    $deviceInfoSet = [SetupAPI]::SetupDiCreateDeviceInfoList([ref]$classGuid, [IntPtr]::Zero)
    
    # Check if handle is invalid (don't try to convert to int32)
    $invalidHandle = [IntPtr]::new(-1)
    if ($deviceInfoSet -eq [IntPtr]::Zero -or $deviceInfoSet -eq $invalidHandle) {
        $errorCode = [System.Runtime.InteropServices.Marshal]::GetLastWin32Error()
        Write-Host "FAILED: SetupDiCreateDeviceInfoList (error: $errorCode)" -ForegroundColor Red
    } else {
        Write-Host "OK: DeviceInfoSet created" -ForegroundColor Green
        
        # Create device info data
        $devInfoData = New-Object SetupAPI+SP_DEVINFO_DATA
        $devInfoData.cbSize = [System.Runtime.InteropServices.Marshal]::SizeOf($devInfoData)
        
        Write-Host "Step 2: Create device info..." -ForegroundColor Cyan
        $result = [SetupAPI]::SetupDiCreateDeviceInfo(
            $deviceInfoSet,
            "IddSampleDriver",
            [ref]$classGuid,
            "IddSampleDriver Device",
            [IntPtr]::Zero,
            [SetupAPI]::DICD_GENERATE_ID,
            [ref]$devInfoData
        )
        
        if ($result) {
            Write-Host "OK: Device info created" -ForegroundColor Green
            
            Write-Host "Step 3: Set hardware ID..." -ForegroundColor Cyan
            $hwId = "ROOT\IddSampleDriver`0`0"
            $hwIdBytes = [System.Text.Encoding]::Unicode.GetBytes($hwId)
            
            $result = [SetupAPI]::SetupDiSetDeviceRegistryProperty(
                $deviceInfoSet,
                [ref]$devInfoData,
                [SetupAPI]::SPDRP_HARDWAREID,
                $hwIdBytes,
                $hwIdBytes.Length
            )
            
            if ($result) {
                Write-Host "OK: Hardware ID set" -ForegroundColor Green
                
                Write-Host "Step 4: Register device..." -ForegroundColor Cyan
                $result = [SetupAPI]::SetupDiCallClassInstaller(
                    [SetupAPI]::DIF_REGISTERDEVICE,
                    $deviceInfoSet,
                    [ref]$devInfoData
                )
                
                if ($result) {
                    Write-Host "OK: Device registered" -ForegroundColor Green
                    
                    Write-Host "Step 5: Install device..." -ForegroundColor Cyan
                    $result = [SetupAPI]::SetupDiCallClassInstaller(
                        [SetupAPI]::DIF_INSTALLDEVICE,
                        $deviceInfoSet,
                        [ref]$devInfoData
                    )
                    
                    if ($result) {
                        Write-Host "SUCCESS: Device installed!" -ForegroundColor Green
                    } else {
                        $errorCode = [System.Runtime.InteropServices.Marshal]::GetLastWin32Error()
                        Write-Host "FAILED: DIF_INSTALLDEVICE (error: $errorCode)" -ForegroundColor Red
                    }
                } else {
                    $errorCode = [System.Runtime.InteropServices.Marshal]::GetLastWin32Error()
                    Write-Host "FAILED: DIF_REGISTERDEVICE (error: $errorCode)" -ForegroundColor Red
                }
            } else {
                $errorCode = [System.Runtime.InteropServices.Marshal]::GetLastWin32Error()
                Write-Host "FAILED: SetupDiSetDeviceRegistryProperty (error: $errorCode)" -ForegroundColor Red
            }
        } else {
            $errorCode = [System.Runtime.InteropServices.Marshal]::GetLastWin32Error()
            Write-Host "FAILED: SetupDiCreateDeviceInfo (error: $errorCode)" -ForegroundColor Red
        }
        
        # Cleanup
        [SetupAPI]::SetupDiDestroyDeviceInfoList($deviceInfoSet) | Out-Null
    }
} catch {
    Write-Host "EXCEPTION: $_" -ForegroundColor Red
}

Write-Host ""

# ============================================================================
# Verify installation
# ============================================================================

Write-Host "[Verification]" -ForegroundColor Yellow
Write-Host "========================================" -ForegroundColor Cyan

Write-Host "Checking device manager..." -ForegroundColor Cyan
try {
    $devices = Get-PnpDevice | Where-Object { 
        $_.InstanceId -like "*IddSampleDriver*" 
    }
    
    if ($devices) {
        Write-Host "OK: Found IddSampleDriver device(s):" -ForegroundColor Green
        foreach ($device in $devices) {
            Write-Host "  - $($device.FriendlyName)" -ForegroundColor White
            Write-Host "    Status: $($device.Status)" -ForegroundColor Gray
            Write-Host "    InstanceId: $($device.InstanceId)" -ForegroundColor Gray
        }
    } else {
        Write-Host "WARNING: No IddSampleDriver device found" -ForegroundColor Yellow
    }
} catch {
    Write-Host "ERROR: $_" -ForegroundColor Red
}

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Installation Complete" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan

