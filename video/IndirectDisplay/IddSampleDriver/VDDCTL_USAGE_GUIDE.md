# VDDCtl Usage Guide

## ✅ Key Facts

### 1. Install/Uninstall ARE INDEPENDENT
**YES!** `install` and `uninstall` work **WITHOUT** calling `init` first.

```powershell
# These work independently - NO init needed!
vddctl install x64\Release\IddSampleDriver\IddSampleDriver.inf
vddctl uninstall
```

**Why?** The code automatically creates a temporary SDK instance:
```cpp
// From vddsdk.cpp line 182-204
Status InstallDriver(const std::wstring& infPath) {
    // InstallDriver doesn't need SDK instance - it's a standalone operation
    // Create temporary instance if needed
    if (!g_sdkInstance) {
        g_sdkInstance = std::make_unique<VddSdkImpl>();
    }
    return g_sdkInstance->InstallDriver(infPath);
}
```

### 2. Works in ANY Command Prompt
**NO VS 2022 CMD Required!** Works in:
- ✅ Standard Windows PowerShell
- ✅ Windows CMD
- ✅ PowerShell 7
- ✅ VS Developer Command Prompt (also works here, but NOT required)

## Command Categories

### Standalone Commands (No init required)
```powershell
vddctl install <inf_path>    # Install driver
vddctl uninstall             # Uninstall driver
vddctl version               # Show version
vddctl help                  # Show help
```

### SDK Session Commands (Init recommended, but optional)
```powershell
vddctl init                  # Initialize SDK session
vddctl status                # Show status
vddctl list                  # List displays
vddctl activate              # Activate display
vddctl deactivate            # Deactivate display
vddctl setmode               # Set display mode
vddctl setlocation           # Set position
vddctl setprimary            # Set as primary
vddctl shutdown              # Shutdown SDK
```

## Complete Usage Examples

### Quick Install/Uninstall (No init needed)
```powershell
# PowerShell or CMD (Run as Administrator)
cd C:\path\to\IddSampleDriver

# Install
.\build\bin\Release\vddctl.exe install x64\Release\IddSampleDriver\IddSampleDriver.inf

# Verify
Get-PnpDevice | Where-Object { $_.InstanceId -like '*IddSampleDriver*' }

# Uninstall
.\build\bin\Release\vddctl.exe uninstall
```

### Full SDK Workflow (With init)
```powershell
# PowerShell or CMD (Run as Administrator)

# 1. Initialize SDK
.\build\bin\Release\vddctl.exe init

# 2. Install driver
.\build\bin\Release\vddctl.exe install x64\Release\IddSampleDriver\IddSampleDriver.inf

# 3. Check status
.\build\bin\Release\vddctl.exe status

# 4. List displays
.\build\bin\Release\vddctl.exe list

# 5. Activate virtual display (if implemented)
.\build\bin\Release\vddctl.exe activate --name "VDD" --width 1920 --height 1080

# 6. Deactivate
.\build\bin\Release\vddctl.exe deactivate

# 7. Uninstall
.\build\bin\Release\vddctl.exe uninstall

# 8. Shutdown SDK
.\build\bin\Release\vddctl.exe shutdown
```

## Design Document Compliance

According to the design doc:
```
vdd-ctl init
vdd-ctl install <inf>
vdd-ctl status
vdd-ctl list
```

**Current Implementation:**
- ✅ `init` - Implemented (optional for install/uninstall)
- ✅ `install` - Fully implemented with all GPT fixes
- ✅ `uninstall` - Fully implemented
- ✅ `status` - Implemented
- ✅ `list` - Implemented

## Testing

### Test 1: Minimal (No init)
```powershell
# Run as Administrator
vddctl install x64\Release\IddSampleDriver\IddSampleDriver.inf
Get-PnpDevice | Where-Object { $_.InstanceId -like '*IddSampleDriver*' }
vddctl uninstall
```

### Test 2: Complete (With init)
```powershell
# Run as Administrator
vddctl init
vddctl status
vddctl install x64\Release\IddSampleDriver\IddSampleDriver.inf
vddctl status
vddctl list
vddctl uninstall
vddctl status
vddctl shutdown
```

### Test 3: Verify in Device Manager
```powershell
# After install
Get-PnpDevice | Where-Object { $_.InstanceId -like '*IddSampleDriver*' } | 
  Format-Table Status, Class, FriendlyName, InstanceId -AutoSize

# Check ProblemCode
$device = Get-PnpDevice | Where-Object { $_.InstanceId -like '*IddSampleDriver*' }
if ($device) {
    $prob = (Get-PnpDeviceProperty -InstanceId $device.InstanceId `
            -KeyName 'DEVPKEY_Device_ProblemCode').Data
    Write-Host "ProblemCode: $prob" -ForegroundColor $(if ($prob -eq 0) { "Green" } else { "Red" })
}
```

## Summary

| Feature | Status | Notes |
|---------|--------|-------|
| `install` independent | ✅ YES | No init required |
| `uninstall` independent | ✅ YES | No init required |
| Works in standard PowerShell | ✅ YES | No VS CMD needed |
| Works in standard CMD | ✅ YES | Any command prompt |
| Admin privileges | ⚠️ Required | For install/uninstall only |
| `init` required? | ❌ NO | Optional for session management |

**Bottom Line:**
- 🎯 Install/Uninstall work standalone
- 🎯 Works in ANY command prompt
- 🎯 Just needs Administrator privileges


## ✅ Key Facts

### 1. Install/Uninstall ARE INDEPENDENT
**YES!** `install` and `uninstall` work **WITHOUT** calling `init` first.

```powershell
# These work independently - NO init needed!
vddctl install x64\Release\IddSampleDriver\IddSampleDriver.inf
vddctl uninstall
```

**Why?** The code automatically creates a temporary SDK instance:
```cpp
// From vddsdk.cpp line 182-204
Status InstallDriver(const std::wstring& infPath) {
    // InstallDriver doesn't need SDK instance - it's a standalone operation
    // Create temporary instance if needed
    if (!g_sdkInstance) {
        g_sdkInstance = std::make_unique<VddSdkImpl>();
    }
    return g_sdkInstance->InstallDriver(infPath);
}
```

### 2. Works in ANY Command Prompt
**NO VS 2022 CMD Required!** Works in:
- ✅ Standard Windows PowerShell
- ✅ Windows CMD
- ✅ PowerShell 7
- ✅ VS Developer Command Prompt (also works here, but NOT required)

## Command Categories

### Standalone Commands (No init required)
```powershell
vddctl install <inf_path>    # Install driver
vddctl uninstall             # Uninstall driver
vddctl version               # Show version
vddctl help                  # Show help
```

### SDK Session Commands (Init recommended, but optional)
```powershell
vddctl init                  # Initialize SDK session
vddctl status                # Show status
vddctl list                  # List displays
vddctl activate              # Activate display
vddctl deactivate            # Deactivate display
vddctl setmode               # Set display mode
vddctl setlocation           # Set position
vddctl setprimary            # Set as primary
vddctl shutdown              # Shutdown SDK
```

## Complete Usage Examples

### Quick Install/Uninstall (No init needed)
```powershell
# PowerShell or CMD (Run as Administrator)
cd C:\path\to\IddSampleDriver

# Install
.\build\bin\Release\vddctl.exe install x64\Release\IddSampleDriver\IddSampleDriver.inf

# Verify
Get-PnpDevice | Where-Object { $_.InstanceId -like '*IddSampleDriver*' }

# Uninstall
.\build\bin\Release\vddctl.exe uninstall
```

### Full SDK Workflow (With init)
```powershell
# PowerShell or CMD (Run as Administrator)

# 1. Initialize SDK
.\build\bin\Release\vddctl.exe init

# 2. Install driver
.\build\bin\Release\vddctl.exe install x64\Release\IddSampleDriver\IddSampleDriver.inf

# 3. Check status
.\build\bin\Release\vddctl.exe status

# 4. List displays
.\build\bin\Release\vddctl.exe list

# 5. Activate virtual display (if implemented)
.\build\bin\Release\vddctl.exe activate --name "VDD" --width 1920 --height 1080

# 6. Deactivate
.\build\bin\Release\vddctl.exe deactivate

# 7. Uninstall
.\build\bin\Release\vddctl.exe uninstall

# 8. Shutdown SDK
.\build\bin\Release\vddctl.exe shutdown
```

## Design Document Compliance

According to the design doc:
```
vdd-ctl init
vdd-ctl install <inf>
vdd-ctl status
vdd-ctl list
```

**Current Implementation:**
- ✅ `init` - Implemented (optional for install/uninstall)
- ✅ `install` - Fully implemented with all GPT fixes
- ✅ `uninstall` - Fully implemented
- ✅ `status` - Implemented
- ✅ `list` - Implemented

## Testing

### Test 1: Minimal (No init)
```powershell
# Run as Administrator
vddctl install x64\Release\IddSampleDriver\IddSampleDriver.inf
Get-PnpDevice | Where-Object { $_.InstanceId -like '*IddSampleDriver*' }
vddctl uninstall
```

### Test 2: Complete (With init)
```powershell
# Run as Administrator
vddctl init
vddctl status
vddctl install x64\Release\IddSampleDriver\IddSampleDriver.inf
vddctl status
vddctl list
vddctl uninstall
vddctl status
vddctl shutdown
```

### Test 3: Verify in Device Manager
```powershell
# After install
Get-PnpDevice | Where-Object { $_.InstanceId -like '*IddSampleDriver*' } | 
  Format-Table Status, Class, FriendlyName, InstanceId -AutoSize

# Check ProblemCode
$device = Get-PnpDevice | Where-Object { $_.InstanceId -like '*IddSampleDriver*' }
if ($device) {
    $prob = (Get-PnpDeviceProperty -InstanceId $device.InstanceId `
            -KeyName 'DEVPKEY_Device_ProblemCode').Data
    Write-Host "ProblemCode: $prob" -ForegroundColor $(if ($prob -eq 0) { "Green" } else { "Red" })
}
```

## Summary

| Feature | Status | Notes |
|---------|--------|-------|
| `install` independent | ✅ YES | No init required |
| `uninstall` independent | ✅ YES | No init required |
| Works in standard PowerShell | ✅ YES | No VS CMD needed |
| Works in standard CMD | ✅ YES | Any command prompt |
| Admin privileges | ⚠️ Required | For install/uninstall only |
| `init` required? | ❌ NO | Optional for session management |

**Bottom Line:**
- 🎯 Install/Uninstall work standalone
- 🎯 Works in ANY command prompt
- 🎯 Just needs Administrator privileges



