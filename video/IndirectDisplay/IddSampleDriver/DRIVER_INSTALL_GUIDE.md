# Driver Installation Tool Usage Guide

## 📦 Tool List

| Tool | Description | Features |
|------|------|------|
| `install_driver.exe` | Install driver (with rollback) | ✅ Auto install<br>✅ State backup<br>✅ Auto rollback on failure<br>✅ Verify installation |
| `uninstall_driver.exe` | Uninstall driver | ✅ Complete device removal<br>✅ Clean driver store<br>✅ Clean registry<br>✅ Verify uninstall |

## 🚀 Quick Start

### Install Driver

```batch
# Run as administrator
install_driver.exe
```

Or specify INF file path:
```batch
install_driver.exe path\to\driver.inf
```

**Installation Process:**
1. 💾 Backup current system state to `driver_install_backup.txt`
2. 🔨 Create virtual display device node
3. 📝 Register device to system
4. 💿 Install driver files
5. ✅ Verify installation success

**If Installation Fails:**
- Tool will ask if you want to rollback
- Choose **Y** to automatically undo all changes
- System will restore to pre-installation state

### Uninstall Driver

```batch
# Run as administrator
uninstall_driver.exe
```

**Uninstall Process:**
1. 🔍 Search for all IddSampleDriver devices
2. 🗑️ Remove devices from Device Manager
3. 🧹 Delete driver from driver store
4. 🔧 Clean registry entries
5. ✅ Verify uninstall complete

## 📋 Installation Checklist

### Before Installation
- [ ] Run as administrator
- [ ] INF file in current directory or specified path
- [ ] Close all apps that might use display driver

### Post-Installation Verification
```powershell
# Check Device Manager
Get-PnpDevice | Where-Object { $_.FriendlyName -like "*IddSampleDriver*" }

# Check driver store
pnputil /enum-drivers | findstr /i "iddsampledriver"

# Check registry
Test-Path "HKLM:\SYSTEM\CurrentControlSet\Enum\ROOT\IddSampleDriver"
```

## 🔄 Rollback Mechanism

### Auto Rollback (On Installation Failure)
When any step fails during installation, tool will ask if you want to rollback:

```
❌ Installation failed!

Do you want to rollback changes? (Y/N): Y

========================================
Performing rollback...
========================================

[Rollback] Removing registered device...
  ✓ Device removed
[Rollback] Cleaning registry entries...
  Complete

✓ Rollback successful
```

### Manual Uninstall
If you need to completely remove driver:

```batch
uninstall_driver.exe
```

## 📁 Backup Files

### driver_install_backup.txt
Auto-created system state backup before installation:

```ini
[InstallState]
Timestamp=2025-01-15 10:30:45

[ExistingDevices]
Device0_ID=PCI\VEN_80EE&DEV_BEEF...
Device0_Desc=VirtualBox Graphics Adapter
DeviceCount=1
```

**Purpose:**
- Record display device state before installation
- Provide rollback reference
- Troubleshooting basis

## 🛠️ Recompile Tools

If you need to modify source code:

```batch
# Compile all tools
build_install_tools.bat
```

**Source files:**
- `setupapi_install_with_rollback.cpp` - Installation tool (with rollback)
- `setupapi_uninstall.cpp` - Uninstall tool

## ⚠️ Common Issues

### 1. Installation Failed: ERROR_NO_MORE_ITEMS (259)
**Cause:** Device doesn't exist, UpdateDriverForPlugAndPlayDevices can't find device

**Solution:** Tool with rollback will automatically create device node

### 2. Installation Failed: ERROR 1004
**Cause:** INF file format issue or path error

**Solution:**
- Check if INF file path is correct
- Confirm INF file format is correct
- Use full path

### 3. Device Still Exists After Uninstall
**Cause:** System cache or service not updated

**Solution:**
```batch
# Rescan hardware
pnputil /scan-devices

# Or restart system
shutdown /r /t 0
```

### 4. Administrator Privileges Required
**Error:** "ERROR: Administrator privileges required"

**Solution:**
- Right-click program → Run as administrator
- Or execute in administrator command prompt

## 🔍 Advanced Operations

### View Detailed Logs
Tool outputs detailed operation logs, can redirect to file:

```batch
install_driver.exe > install_log.txt 2>&1
uninstall_driver.exe > uninstall_log.txt 2>&1
```

### Batch Operations
Create batch script to automate install/uninstall:

```batch
@echo off
echo Installing driver...
install_driver.exe IddSampleDriver_Fixed.inf

if %ERRORLEVEL% EQU 0 (
    echo Installation successful
) else (
    echo Installation failed, performing cleanup...
    uninstall_driver.exe
)
```

### Integration with VDD SDK
After installation, can use VDD SDK to control virtual display:

```cpp
#include "vddsdk.h"

// Initialize SDK
vdd::Initialize();

// Activate virtual display
vdd::VirtualDisplayDesc desc;
desc.name = "My Virtual Display";
desc.preferredMode.width = 1920;
desc.preferredMode.height = 1080;
vdd::Activate(desc, 1);
```

## 📞 Support

If you encounter issues:

1. Check `driver_install_backup.txt` to understand pre-installation state
2. View error messages and codes output by tool
3. Check device status in Device Manager
4. Try running `uninstall_driver.exe` for complete cleanup then reinstall

## 📝 Version History

### v1.0 (2025-01)
- ✅ SetupAPI driver installation
- ✅ Auto rollback mechanism
- ✅ State backup function
- ✅ Complete uninstall tool
- ✅ Detailed error reporting

---

**Security Tip:** Installing and uninstalling drivers requires administrator privileges. Ensure driver files are obtained from trusted sources.










