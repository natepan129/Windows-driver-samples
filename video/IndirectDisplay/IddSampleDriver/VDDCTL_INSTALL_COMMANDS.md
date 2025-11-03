# VDD Control Tool - Install Commands

## 🚀 Using vddctl.exe to Install IddSampleDriver

---

## Basic Installation Command

### Command Format
```batch
vddctl install --inf <INF file path>
```

### Simplest Installation
```batch
vddctl install --inf IddSampleDriver_Fixed.inf
```

---

## ✅ Complete Installation Process

### Method 1: Manual Step-by-Step

```batch
REM Step 1: Initialize SDK
vddctl init

REM Step 2: Install driver (requires administrator privileges, will show UAC)
vddctl install --inf IddSampleDriver_Fixed.inf

REM Step 3: Verify installation
vddctl status
```

### Method 2: Use Script (Recommended)

```batch
cmd /c install_with_vddctl.bat
```

### Method 3: Run with Administrator Privileges

```batch
REM Run as administrator in PowerShell
powershell -Command "Start-Process -FilePath 'build\bin\Release\vddctl.exe' -ArgumentList 'install --inf IddSampleDriver_Fixed.inf' -Verb RunAs -Wait"
```

---

## 📋 Detailed Steps

### 1. Prepare Files

Ensure following files exist:
- ✅ `build\bin\Release\vddctl.exe` (compiled tool)
- ✅ `IddSampleDriver_Fixed.inf` (driver INF file)
- ✅ `x64\Release\IddSampleDriver.dll` (driver DLL)

### 2. Initialize SDK

```batch
C:\Path\To\Project> vddctl init
```

**Output**:
```
Initializing VDD SDK...
VDD SDK initialized successfully.
```

### 3. Install Driver

```batch
C:\Path\To\Project> vddctl install --inf IddSampleDriver_Fixed.inf
```

**Output**:
```
Installing driver...
Driver installed successfully.
```

⚠️ **Note**: 
- Will show UAC prompt, click "Yes" to allow
- If old device exists, will automatically update

### 4. Verify Installation

```batch
C:\Path\To\Project> vddctl status
```

**Successful Output**:
```
VDD SDK Status:
==============
Driver Installed: Yes
Driver Version: 1.0.0
Display Active: No
System Info: Windows 10/11 x64
```

---

## 🔍 Verify Installation Success

### Check with vddctl
```batch
vddctl status
```
Should display: `Driver Installed: Yes`

### Check with PowerShell
```powershell
Get-PnpDevice | Where-Object { $_.FriendlyName -like '*IddSampleDriver*' }
```

**Successful Output**:
```
FriendlyName           Status Class   ClassGuid
------------           ------ -----   ---------
IddSampleDriver Device OK     Display {4d36e968-e325-11ce-bfc1-08002be10318}
```

### Check in Device Manager
1. Press `Win + X` to open Device Manager
2. Expand "Display adapters"
3. Should see "IddSampleDriver Device"

---

## 🛠️ Complete Example Scripts

### simple_install.bat
```batch
@echo off
echo Installing IddSampleDriver with vddctl...

REM Initialize
vddctl init

REM Install (will show UAC prompt)
vddctl install --inf IddSampleDriver_Fixed.inf

REM Verify
vddctl status

echo.
echo Installation complete!
pause
```

### install_with_admin.bat
```batch
@echo off
echo Installing IddSampleDriver with admin rights...

set VDDCTL=%CD%\build\bin\Release\vddctl.exe
set INF_PATH=%CD%\IddSampleDriver_Fixed.inf

REM Initialize
"%VDDCTL%" init

REM Install with admin rights
powershell -Command "Start-Process '%VDDCTL%' -ArgumentList 'install --inf \"%INF_PATH%\"' -Verb RunAs -Wait"

REM Verify
"%VDDCTL%" status

pause
```

---

## ❌ Common Errors and Solutions

### Error 1: "vddctl.exe not found"

**Cause**: vddctl not compiled

**Solution**:
```batch
cmd /c compile_sdk_only.bat
```

### Error 2: "INF file not found"

**Cause**: Incorrect INF path

**Solution**:
```batch
REM Use absolute path
vddctl install --inf "C:\Full\Path\To\IddSampleDriver_Fixed.inf"
```

### Error 3: "Access Denied" (Error 5)

**Cause**: No administrator privileges

**Solution**:
```batch
REM Right-click cmd/PowerShell, select "Run as administrator"
REM Or use PowerShell to elevate privileges:
powershell -Command "Start-Process 'vddctl.exe' -ArgumentList 'install --inf IddSampleDriver_Fixed.inf' -Verb RunAs -Wait"
```

### Error 4: "Driver Installed: No" (After Installation)

**Cause**: Installation may have failed

**Solution**:
```batch
REM 1. Check INF file encoding (should be UTF-16 LE or ANSI)
REM 2. Check error log
vddctl install --inf IddSampleDriver_Fixed.inf

REM 3. Verify files
dir IddSampleDriver_Fixed.inf
dir x64\Release\IddSampleDriver.dll
```

### Error 5: Device Has No Display Class

**Cause**: 
- INF file error
- Used wrong INF (UTF-8 vs UTF-16)

**Solution**:
```batch
REM Use IddSampleDriver_Fixed.inf (verified working)
vddctl uninstall
vddctl install --inf IddSampleDriver_Fixed.inf
```

---

## 📊 Command Comparison

| Operation | install_driver.exe | vddctl.exe |
|------|-------------------|------------|
| Install command | `install_driver.exe IddSampleDriver_Fixed.inf` | `vddctl install --inf IddSampleDriver_Fixed.inf` |
| Requires admin | ✅ Yes | ✅ Yes |
| View status support | ❌ No | ✅ Yes (`vddctl status`) |
| Uninstall support | ❌ No (needs uninstall_driver.exe) | ✅ Yes (`vddctl uninstall`) |
| Display activation support | ❌ No | ✅ Yes (`vddctl activate`) |
| Design doc recommended | ❌ No | ✅ Yes |

**Recommended Use**: `vddctl.exe` (more complete functionality, official design doc tool)

---

## 🎯 Quick Reference

### Install
```batch
vddctl install --inf IddSampleDriver_Fixed.inf
```

### Uninstall
```batch
vddctl uninstall
```

### Check Status
```batch
vddctl status
```

### Activate Display
```batch
vddctl activate --width 1920 --height 1080
```

### Deactivate Display
```batch
vddctl deactivate
```

---

## 📝 Complete Workflow

```batch
@echo off
REM ========================================
REM Complete IddSampleDriver Installation
REM ========================================

REM 1. Initialize SDK
echo [1/6] Initializing SDK...
vddctl init

REM 2. Check current status
echo [2/6] Checking status...
vddctl status

REM 3. Install driver
echo [3/6] Installing driver...
vddctl install --inf IddSampleDriver_Fixed.inf

REM 4. Verify installation
echo [4/6] Verifying installation...
vddctl status

REM 5. Activate virtual display
echo [5/6] Activating virtual display...
vddctl activate --width 1920 --height 1080

REM 6. List all displays
echo [6/6] Listing displays...
vddctl list

echo.
echo Installation and activation complete!
pause
```

---

## ⚡ One-Click Installation Script

Create file `quick_install_vddctl.bat`:

```batch
@echo off
set VDDCTL=build\bin\Release\vddctl.exe
set INF=IddSampleDriver_Fixed.inf

echo Installing IddSampleDriver...
"%VDDCTL%" init
powershell -Command "Start-Process '%VDDCTL%' -ArgumentList 'install --inf %INF%' -Verb RunAs -Wait"
"%VDDCTL%" status
echo Done!
pause
```

Run:
```batch
cmd /c quick_install_vddctl.bat
```

---

## 🔗 Related Documentation

- **Complete Command Reference**: `VDDCTL_COMMAND_REFERENCE.md`
- **Basic Functionality Verification**: `VDDSDK_BASIC_VERIFICATION.md`
- **Test Script**: `install_with_vddctl.bat`




