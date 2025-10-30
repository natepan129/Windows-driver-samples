# VDD Control Tool (vddctl) - Basic Testing Guide

## 📋 Overview

`vddctl.exe` is the command-line interface from the design document for managing the VDD (Virtual Display Driver).

**Location**: `build\bin\Release\vddctl.exe`

---

## ✅ Available Commands

### Core Functions

```bash
vddctl init                      # Initialize SDK
vddctl shutdown                  # Shutdown SDK
vddctl status                    # Show current status
vddctl version                   # Show version info
```

### Driver Management

```bash
vddctl install --inf <path>      # Install driver
vddctl uninstall                 # Uninstall driver
```

### Display Control

```bash
vddctl activate                  # Activate virtual display
vddctl deactivate                # Deactivate virtual display
vddctl list                      # List displays and adapters
```

### Display Configuration

```bash
vddctl setmode --index <n>       # Set display mode
vddctl setlocation --index <n>   # Set display location
vddctl setprimary --index <n>    # Set primary display
```

---

## 🧪 Basic Function Tests

### Test 1: Verify Tool Works

```batch
build\bin\Release\vddctl.exe version
build\bin\Release\vddctl.exe status
```

**Expected Output**:
```
VDD SDK Version: 1.0.0
Build Date: ...

VDD SDK Status:
==============
Driver Installed: No/Yes
Display Active: No/Yes
```

✅ **Pass Criteria**: Tool runs without crashing, shows version and status

---

### Test 2: Initialize SDK

```batch
build\bin\Release\vddctl.exe init --verbose
```

**Expected Output**:
```
Initializing VDD SDK...
VDD SDK initialized successfully.
```

✅ **Pass Criteria**: SDK initializes without errors

---

### Test 3: Install Driver

```batch
build\bin\Release\vddctl.exe install --inf IddSampleDriver_Fixed.inf
```

**Expected Output**:
```
Installing driver...
Driver installed successfully.
```

⚠️ **Note**: Requires administrator rights (UAC prompt will appear)

✅ **Pass Criteria**: 
- Installation completes without errors
- `vddctl status` shows `Driver Installed: Yes`

---

### Test 4: Activate Virtual Display

```batch
build\bin\Release\vddctl.exe activate --width 1920 --height 1080 --refresh 60
```

**Expected Output**:
```
Activating virtual display...
Virtual display activated successfully.
Name: Virtual Display
Resolution: 1920x1080
Refresh Rate: 60Hz
```

✅ **Pass Criteria**:
- Virtual display created
- Visible in Device Manager
- `vddctl status` shows `Display Active: Yes`
- `vddctl list` shows the virtual display

---

### Test 5: Query Display Status

```batch
build\bin\Release\vddctl.exe status
build\bin\Release\vddctl.exe list
```

**Expected Output**:
```
VDD SDK Status:
==============
Driver Installed: Yes
Display Active: Yes
Active Displays: 1

Displays:
  [0] Virtual Display (1920x1080 @ 60Hz)
```

✅ **Pass Criteria**: Can query display information without errors

---

### Test 6: Deactivate Virtual Display

```batch
build\bin\Release\vddctl.exe deactivate
```

**Expected Output**:
```
Deactivating virtual display...
Virtual display deactivated successfully.
```

✅ **Pass Criteria**:
- Virtual display removed
- `vddctl status` shows `Display Active: No`

---

### Test 7: Uninstall Driver

```batch
build\bin\Release\vddctl.exe uninstall
```

**Expected Output**:
```
Uninstalling driver...
Driver uninstalled successfully.
```

⚠️ **Note**: Requires administrator rights

✅ **Pass Criteria**:
- Driver removed from system
- `vddctl status` shows `Driver Installed: No`
- No devices in Device Manager

---

## 🚀 Quick Test Scripts

### Automated Test Script

```batch
cmd /c test_vddctl_basic.bat        # Basic function tests
cmd /c test_vddctl_complete.bat     # Complete installation test
```

### Manual Test Sequence

```batch
REM 1. Check status
build\bin\Release\vddctl.exe status

REM 2. Initialize
build\bin\Release\vddctl.exe init

REM 3. Install (needs admin)
build\bin\Release\vddctl.exe install --inf IddSampleDriver_Fixed.inf

REM 4. Activate
build\bin\Release\vddctl.exe activate --width 1920 --height 1080

REM 5. Check it works
build\bin\Release\vddctl.exe list

REM 6. Deactivate
build\bin\Release\vddctl.exe deactivate

REM 7. Uninstall (needs admin)
build\bin\Release\vddctl.exe uninstall
```

---

## ✅ Success Criteria

### Minimum Requirements

To confirm basic functionality is working and safe:

- [ ] **vddctl runs without crashing**
  - Version and help commands work
  - Status command works

- [ ] **SDK initialization works**
  - `vddctl init` succeeds
  - No errors in initialization

- [ ] **Driver can be installed**
  - Installation completes successfully
  - Driver visible in Device Manager
  - Has correct Display class

- [ ] **Virtual display can be created**
  - `vddctl activate` succeeds
  - Display appears in system
  - Can be detected by Windows

- [ ] **Virtual display can be removed**
  - `vddctl deactivate` succeeds
  - Display disappears from system
  - No orphaned devices

- [ ] **System remains stable**
  - No crashes during operations
  - No blue screens
  - System responsive

- [ ] **Clean uninstall works**
  - Driver can be fully removed
  - No leftover devices
  - No registry leftovers

---

## 🔍 Troubleshooting

### Issue: "Driver Installed: No" after install

**Cause**: Installation may have failed or requires admin rights

**Solution**:
```batch
REM Run with admin rights
powershell -Command "Start-Process 'build\bin\Release\vddctl.exe' -ArgumentList 'install --inf IddSampleDriver_Fixed.inf' -Verb RunAs -Wait"
```

### Issue: "Display Active: No" after activate

**Cause**: Driver may not be installed or initialized

**Solution**:
```batch
REM 1. Check driver status
build\bin\Release\vddctl.exe status

REM 2. Ensure driver is installed
build\bin\Release\vddctl.exe install --inf IddSampleDriver_Fixed.inf

REM 3. Reinitialize
build\bin\Release\vddctl.exe init

REM 4. Try activate again
build\bin\Release\vddctl.exe activate --width 1920 --height 1080
```

### Issue: "Failed to enumerate adapters"

**Cause**: SDK not initialized or driver not installed

**Solution**:
```batch
build\bin\Release\vddctl.exe init
build\bin\Release\vddctl.exe install --inf IddSampleDriver_Fixed.inf
```

---

## 📊 Test Results Template

```
Test Date: ___________
Tester: ___________

[ ] vddctl version - works
[ ] vddctl status - works
[ ] vddctl init - works
[ ] vddctl install - works (driver installed)
[ ] vddctl activate - works (display created)
[ ] vddctl list - works (shows display)
[ ] vddctl deactivate - works (display removed)
[ ] vddctl uninstall - works (driver removed)
[ ] System stability - no crashes
[ ] Safety - no blue screens

Overall Result: PASS / FAIL
Notes: ________________
```

---

## 🎯 Next Steps

After basic tests pass:

1. **Test advanced features**
   - SetMode (change resolution)
   - SetLocation (move display)
   - SetPrimary (set as primary display)

2. **Test edge cases**
   - Multiple displays
   - Rapid activate/deactivate
   - System stress test

3. **Test error handling**
   - Invalid parameters
   - Driver not installed scenarios
   - Concurrent operations

4. **Performance testing**
   - Activation time
   - Deactivation time
   - System resource usage

