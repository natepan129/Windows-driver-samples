# ============================================================================
# 使用 SetupAPI 安裝 UMDF 驅動程式的 PowerShell 腳本
# ============================================================================

param(
    [string]$InfPath = "IddSampleDriver_Fixed.inf",
    [switch]$Verbose
)

# 檢查管理員權限
$isAdmin = ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)

if (-not $isAdmin) {
    Write-Host "錯誤: 需要管理員權限來安裝驅動程式" -ForegroundColor Red
    Write-Host "請以管理員身份運行此腳本" -ForegroundColor Yellow
    exit 1
}

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "使用 SetupAPI 安裝 UMDF 驅動程式" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# 檢查 INF 文件是否存在
if (-not (Test-Path $InfPath)) {
    Write-Host "錯誤: 找不到 INF 文件: $InfPath" -ForegroundColor Red
    exit 1
}

$InfPath = (Resolve-Path $InfPath).Path
Write-Host "INF 文件路徑: $InfPath" -ForegroundColor Green
Write-Host ""

# ============================================================================
# 定義 SetupAPI 相關的 P/Invoke
# ============================================================================

Add-Type -TypeDefinition @"
using System;
using System.Runtime.InteropServices;
using System.Text;

public class SetupAPI {
    // Constants
    public const uint DICD_GENERATE_ID = 0x00000001;
    public const uint DICD_INHERIT_CLASSDRVS = 0x00000002;
    
    public const uint DIGCF_DEFAULT = 0x00000001;
    public const uint DIGCF_PRESENT = 0x00000002;
    public const uint DIGCF_ALLCLASSES = 0x00000004;
    public const uint DIGCF_PROFILE = 0x00000008;
    public const uint DIGCF_DEVICEINTERFACE = 0x00000010;
    
    public const uint SPDRP_DEVICEDESC = 0x00000000;
    public const uint SPDRP_HARDWAREID = 0x00000001;
    public const uint SPDRP_DRIVER = 0x00000009;
    
    public const uint DIF_REGISTERDEVICE = 0x00000019;
    public const uint DIF_INSTALLDEVICE = 0x00000002;
    
    public const uint INSTALLFLAG_FORCE = 0x00000001;
    public const uint INSTALLFLAG_READONLY = 0x00000002;
    public const uint INSTALLFLAG_NONINTERACTIVE = 0x00000004;
    
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
    
    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
    public struct SP_DRVINFO_DATA {
        public uint cbSize;
        public uint DriverType;
        public IntPtr Reserved;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 256)]
        public string Description;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 256)]
        public string MfgName;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 256)]
        public string ProviderName;
        public System.Runtime.InteropServices.ComTypes.FILETIME DriverDate;
        public ulong DriverVersion;
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
    public static extern bool SetupDiDestroyDeviceInfoList(IntPtr DeviceInfoSet);
    
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
    public static extern bool SetupDiBuildDriverInfoList(
        IntPtr DeviceInfoSet,
        ref SP_DEVINFO_DATA DeviceInfoData,
        uint DriverType
    );
    
    [DllImport("setupapi.dll", CharSet = CharSet.Unicode, SetLastError = true)]
    public static extern bool SetupDiSetSelectedDriver(
        IntPtr DeviceInfoSet,
        ref SP_DEVINFO_DATA DeviceInfoData,
        ref SP_DRVINFO_DATA DriverInfoData
    );
    
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
}
"@

# ============================================================================
# 方法 1: 使用 pnputil (最推薦的方法)
# ============================================================================

Write-Host "方法 1: 使用 pnputil 安裝驅動程式" -ForegroundColor Yellow
Write-Host "----------------------------------------" -ForegroundColor Gray

try {
    # 首先添加驅動程式到驅動程式存儲區
    Write-Host "步驟 1: 添加驅動程式到驅動程式存儲區..." -ForegroundColor Cyan
    $addResult = & pnputil.exe /add-driver "$InfPath" /install 2>&1
    
    if ($LASTEXITCODE -eq 0) {
        Write-Host "✓ 驅動程式已成功添加到驅動程式存儲區" -ForegroundColor Green
        Write-Host $addResult
        
        # 列出已安裝的驅動程式
        Write-Host "`n步驟 2: 驗證驅動程式安裝..." -ForegroundColor Cyan
        $enumResult = & pnputil.exe /enum-drivers 2>&1
        
        if ($enumResult -match "iddsampledriver") {
            Write-Host "✓ 驅動程式已在系統中找到" -ForegroundColor Green
        } else {
            Write-Host "⚠ 驅動程式可能未正確安裝" -ForegroundColor Yellow
        }
        
    } else {
        Write-Host "✗ pnputil 安裝失敗 (錯誤碼: $LASTEXITCODE)" -ForegroundColor Red
        Write-Host $addResult
    }
} catch {
    Write-Host "✗ pnputil 執行失敗: $_" -ForegroundColor Red
}

Write-Host ""

# ============================================================================
# 方法 2: 使用 UpdateDriverForPlugAndPlayDevices API
# ============================================================================

Write-Host "方法 2: 使用 UpdateDriverForPlugAndPlayDevices API" -ForegroundColor Yellow
Write-Host "----------------------------------------" -ForegroundColor Gray

try {
    $hwndParent = [IntPtr]::Zero
    $hardwareId = "ROOT\IddSampleDriver"
    $installFlags = [SetupAPI]::INSTALLFLAG_FORCE -bor [SetupAPI]::INSTALLFLAG_NONINTERACTIVE
    $rebootRequired = $false
    
    Write-Host "硬體 ID: $hardwareId" -ForegroundColor Cyan
    Write-Host "INF 路徑: $InfPath" -ForegroundColor Cyan
    
    $result = [SetupAPI]::UpdateDriverForPlugAndPlayDevices(
        $hwndParent,
        $hardwareId,
        $InfPath,
        $installFlags,
        [ref]$rebootRequired
    )
    
    if ($result) {
        Write-Host "✓ UpdateDriverForPlugAndPlayDevices 安裝成功" -ForegroundColor Green
        if ($rebootRequired) {
            Write-Host "⚠ 需要重新啟動系統" -ForegroundColor Yellow
        }
    } else {
        $lastError = [System.Runtime.InteropServices.Marshal]::GetLastWin32Error()
        Write-Host "✗ UpdateDriverForPlugAndPlayDevices 失敗 (錯誤碼: $lastError)" -ForegroundColor Red
        
        # 解釋常見錯誤碼
        switch ($lastError) {
            2 { Write-Host "   錯誤: 找不到指定的文件" -ForegroundColor Red }
            87 { Write-Host "   錯誤: 參數不正確" -ForegroundColor Red }
            259 { Write-Host "   錯誤: 找不到匹配的設備" -ForegroundColor Red }
            1168 { Write-Host "   錯誤: 找不到設備" -ForegroundColor Red }
            default { Write-Host "   Win32 錯誤碼: $lastError" -ForegroundColor Red }
        }
    }
} catch {
    Write-Host "✗ API 調用失敗: $_" -ForegroundColor Red
}

Write-Host ""

# ============================================================================
# 方法 3: 使用 DiInstallDriver API
# ============================================================================

Write-Host "方法 3: 使用 DiInstallDriver API" -ForegroundColor Yellow
Write-Host "----------------------------------------" -ForegroundColor Gray

try {
    $hwndParent = [IntPtr]::Zero
    $flags = [SetupAPI]::INSTALLFLAG_FORCE -bor [SetupAPI]::INSTALLFLAG_NONINTERACTIVE
    $needReboot = $false
    
    Write-Host "INF 路徑: $InfPath" -ForegroundColor Cyan
    
    $result = [SetupAPI]::DiInstallDriver(
        $hwndParent,
        $InfPath,
        $flags,
        [ref]$needReboot
    )
    
    if ($result) {
        Write-Host "✓ DiInstallDriver 安裝成功" -ForegroundColor Green
        if ($needReboot) {
            Write-Host "⚠ 需要重新啟動系統" -ForegroundColor Yellow
        }
    } else {
        $lastError = [System.Runtime.InteropServices.Marshal]::GetLastWin32Error()
        Write-Host "✗ DiInstallDriver 失敗 (錯誤碼: $lastError)" -ForegroundColor Red
    }
} catch {
    Write-Host "✗ API 調用失敗: $_" -ForegroundColor Red
}

Write-Host ""

# ============================================================================
# 方法 4: 創建設備節點並安裝驅動程式
# ============================================================================

Write-Host "方法 4: 使用 SetupDiCreateDeviceInfo 創建設備節點" -ForegroundColor Yellow
Write-Host "----------------------------------------" -ForegroundColor Gray

try {
    # Display adapter class GUID
    $displayClassGuid = [Guid]::Parse("{4D36E968-E325-11CE-BFC1-08002BE10318}")
    
    Write-Host "創建設備信息集..." -ForegroundColor Cyan
    $deviceInfoSet = [SetupAPI]::SetupDiCreateDeviceInfoList([ref]$displayClassGuid, [IntPtr]::Zero)
    
    if ($deviceInfoSet -eq [IntPtr]::Zero -or $deviceInfoSet -eq -1) {
        $lastError = [System.Runtime.InteropServices.Marshal]::GetLastWin32Error()
        Write-Host "✗ 創建設備信息集失敗 (錯誤碼: $lastError)" -ForegroundColor Red
    } else {
        Write-Host "✓ 設備信息集創建成功: $deviceInfoSet" -ForegroundColor Green
        
        # 創建設備信息數據結構
        $devInfoData = New-Object SetupAPI+SP_DEVINFO_DATA
        $devInfoData.cbSize = [System.Runtime.InteropServices.Marshal]::SizeOf($devInfoData)
        
        Write-Host "創建設備信息..." -ForegroundColor Cyan
        $result = [SetupAPI]::SetupDiCreateDeviceInfo(
            $deviceInfoSet,
            "ROOT\IddSampleDriver",
            [ref]$displayClassGuid,
            "IddSampleDriver Device",
            [IntPtr]::Zero,
            [SetupAPI]::DICD_GENERATE_ID,
            [ref]$devInfoData
        )
        
        if ($result) {
            Write-Host "✓ 設備信息創建成功" -ForegroundColor Green
            
            # 設置硬體 ID
            Write-Host "設置硬體 ID..." -ForegroundColor Cyan
            $hardwareId = "ROOT\IddSampleDriver`0`0"
            $hardwareIdBytes = [System.Text.Encoding]::Unicode.GetBytes($hardwareId)
            
            $result = [SetupAPI]::SetupDiSetDeviceRegistryProperty(
                $deviceInfoSet,
                [ref]$devInfoData,
                [SetupAPI]::SPDRP_HARDWAREID,
                $hardwareIdBytes,
                $hardwareIdBytes.Length
            )
            
            if ($result) {
                Write-Host "✓ 硬體 ID 設置成功" -ForegroundColor Green
                
                # 註冊設備
                Write-Host "註冊設備..." -ForegroundColor Cyan
                $result = [SetupAPI]::SetupDiCallClassInstaller(
                    [SetupAPI]::DIF_REGISTERDEVICE,
                    $deviceInfoSet,
                    [ref]$devInfoData
                )
                
                if ($result) {
                    Write-Host "✓ 設備註冊成功" -ForegroundColor Green
                    
                    # 安裝驅動程式
                    Write-Host "安裝驅動程式..." -ForegroundColor Cyan
                    $result = [SetupAPI]::SetupDiCallClassInstaller(
                        [SetupAPI]::DIF_INSTALLDEVICE,
                        $deviceInfoSet,
                        [ref]$devInfoData
                    )
                    
                    if ($result) {
                        Write-Host "✓ 驅動程式安裝成功" -ForegroundColor Green
                    } else {
                        $lastError = [System.Runtime.InteropServices.Marshal]::GetLastWin32Error()
                        Write-Host "✗ 驅動程式安裝失敗 (錯誤碼: $lastError)" -ForegroundColor Red
                    }
                } else {
                    $lastError = [System.Runtime.InteropServices.Marshal]::GetLastWin32Error()
                    Write-Host "✗ 設備註冊失敗 (錯誤碼: $lastError)" -ForegroundColor Red
                }
            } else {
                $lastError = [System.Runtime.InteropServices.Marshal]::GetLastWin32Error()
                Write-Host "✗ 硬體 ID 設置失敗 (錯誤碼: $lastError)" -ForegroundColor Red
            }
        } else {
            $lastError = [System.Runtime.InteropServices.Marshal]::GetLastWin32Error()
            Write-Host "✗ 設備信息創建失敗 (錯誤碼: $lastError)" -ForegroundColor Red
        }
        
        # 清理
        [SetupAPI]::SetupDiDestroyDeviceInfoList($deviceInfoSet) | Out-Null
    }
} catch {
    Write-Host "✗ 設備創建失敗: $_" -ForegroundColor Red
}

Write-Host ""

# ============================================================================
# 驗證安裝
# ============================================================================

Write-Host "驗證驅動程式安裝狀態" -ForegroundColor Yellow
Write-Host "========================================" -ForegroundColor Cyan

# 檢查註冊表
Write-Host "`n檢查註冊表..." -ForegroundColor Cyan

$registryPaths = @(
    "HKLM:\SYSTEM\CurrentControlSet\Services\IddSampleDriver",
    "HKLM:\SYSTEM\CurrentControlSet\Services\WUDFRd",
    "HKLM:\SYSTEM\CurrentControlSet\Enum\ROOT\IddSampleDriver",
    "HKLM:\SYSTEM\CurrentControlSet\Control\Class\{4D36E968-E325-11CE-BFC1-08002BE10318}"
)

foreach ($path in $registryPaths) {
    if (Test-Path $path) {
        Write-Host "✓ 找到: $path" -ForegroundColor Green
    } else {
        Write-Host "✗ 未找到: $path" -ForegroundColor Yellow
    }
}

# 檢查設備管理器
Write-Host "`n檢查設備管理器..." -ForegroundColor Cyan

try {
    $devices = Get-PnpDevice | Where-Object { $_.FriendlyName -like "*IddSampleDriver*" -or $_.InstanceId -like "*IddSampleDriver*" }
    
    if ($devices) {
        Write-Host "✓ 在設備管理器中找到設備:" -ForegroundColor Green
        foreach ($device in $devices) {
            Write-Host "   - $($device.FriendlyName) ($($device.Status))" -ForegroundColor White
        }
    } else {
        Write-Host "✗ 在設備管理器中未找到設備" -ForegroundColor Yellow
    }
} catch {
    Write-Host "✗ 無法查詢設備管理器: $_" -ForegroundColor Red
}

# 檢查驅動程式存儲區
Write-Host "`n檢查驅動程式存儲區..." -ForegroundColor Cyan

try {
    $driverStoreOutput = & pnputil.exe /enum-drivers 2>&1 | Out-String
    
    if ($driverStoreOutput -match "iddsampledriver") {
        Write-Host "✓ 驅動程式在驅動程式存儲區中" -ForegroundColor Green
        
        # 提取相關信息
        $lines = $driverStoreOutput -split "`n"
        $inDriverSection = $false
        foreach ($line in $lines) {
            if ($line -match "iddsampledriver") {
                $inDriverSection = $true
            }
            if ($inDriverSection) {
                Write-Host "   $line" -ForegroundColor White
                if ($line -match "^$") {
                    break
                }
            }
        }
    } else {
        Write-Host "✗ 驅動程式不在驅動程式存儲區中" -ForegroundColor Yellow
    }
} catch {
    Write-Host "✗ 無法檢查驅動程式存儲區: $_" -ForegroundColor Red
}

Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host "安裝腳本執行完成" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan


