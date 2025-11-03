# VDD Control Tool (vddctl) - Command Reference

## 📋 Overview

`vddctl` is the VDD SDK command-line tool for managing virtual display drivers.

**Location**: `build\bin\Release\vddctl.exe`

---

## 🚀 Basic Commands

### 1. `init` - Initialize SDK

Initialize VDD SDK, prepare driver management environment.

#### Syntax
```bash
vddctl init [--verbose]
```

#### Options
- `--verbose`: Display detailed log output

#### Examples
```batch
REM Basic initialization
vddctl init

REM Detailed log initialization
vddctl init --verbose
```

#### Output
```
Initializing VDD SDK...
VDD SDK initialized successfully.
```

#### Return Value
- `0`: Success
- `Non-zero`: Failed (check error message)

---

### 2. `status` - View Status

Display current status of VDD SDK and driver.

#### Syntax
```bash
vddctl status
```

#### Examples
```batch
vddctl status
```

#### Output
```
VDD SDK Status:
==============
Driver Installed: Yes/No
Display Active: Yes/No
System Info: [system information]
```

If driver is installed, will also display:
```
Driver Version: 1.0.0
Active Displays: 2
```

---

### 3. `version` - View Version

Display VDD SDK version information.

#### Syntax
```bash
vddctl version
```

#### Examples
```batch
vddctl version
```

#### Output
```
VDD SDK Version: 1.0.0
Build Date: Oct 27 2025 14:24:13
```

---

### 4. `install` - Install Driver

Install virtual display driver to system.

#### Syntax
```bash
vddctl install --inf <INF file path>
```

#### Options
- `--inf <path>`: Full path to INF file (required)

#### Examples
```batch
REM Use relative path
vddctl install --inf IddSampleDriver_Fixed.inf

REM Use absolute path
vddctl install --inf "C:\Path\To\IddSampleDriver_Fixed.inf"
```

#### Output
```
Installing driver...
Driver installed successfully.
```

#### Notes
- ⚠️ **Requires administrator privileges** (will show UAC prompt)
- Use `IddSampleDriver_Fixed.inf` (UTF-8 encoding)
- DLL from `x64\Release\IddSampleDriver.dll`

---

### 5. `uninstall` - Uninstall Driver

Uninstall virtual display driver from system.

#### Syntax
```bash
vddctl uninstall
```

#### Examples
```batch
vddctl uninstall
```

#### Output
```
Uninstalling driver...
Driver uninstalled successfully.
```

#### Notes
- ⚠️ **Requires administrator privileges**
- Will remove all IddSampleDriver devices
- Will cleanup related registry entries

---

### 6. `activate` - Activate Virtual Display

Create and activate virtual display.

#### Syntax
```bash
vddctl activate [options]
```

#### Options
- `--name <name>`: Display name (default: "Virtual Display")
- `--width <width>`: Width (default: 1920)
- `--height <height>`: Height (default: 1080)
- `--refresh <rate>`: Refresh rate (default: 60)
- `--count <number>`: Number to create (default: 1)
- `--hdr`: Enable HDR10
- `--stereo`: Enable stereo display

#### Examples
```batch
REM Basic activation (1920x1080 @ 60Hz)
vddctl activate

REM Custom resolution
vddctl activate --width 2560 --height 1440 --refresh 75

REM Create multiple displays
vddctl activate --width 1920 --height 1080 --count 2

REM Enable HDR
vddctl activate --width 3840 --height 2160 --hdr

REM Custom name
vddctl activate --name "My Virtual Display" --width 1920 --height 1080
```

#### Output
```
Activating virtual display...
Virtual display activated successfully.
Name: Virtual Display
Resolution: 1920x1080
Refresh Rate: 60Hz
Count: 1
```

---

### 7. `deactivate` - Deactivate Virtual Display

Remove all virtual displays.

#### Syntax
```bash
vddctl deactivate
```

#### Examples
```batch
vddctl deactivate
```

#### Output
```
Deactivating virtual display...
Virtual display deactivated successfully.
```

---

### 8. `setmode` - Set Display Mode

Change resolution and refresh rate of virtual display.

#### Syntax
```bash
vddctl setmode --index <index> --width <width> --height <height> [--refresh <rate>]
```

#### Options
- `--index <n>`: Display index (0-based)
- `--width <width>`: New width
- `--height <height>`: New height
- `--refresh <rate>`: New refresh rate (default: 60)

#### Examples
```batch
REM Change first display to 2560x1440 @ 60Hz
vddctl setmode --index 0 --width 2560 --height 1440

REM Change refresh rate
vddctl setmode --index 0 --width 1920 --height 1080 --refresh 120

REM Change second display
vddctl setmode --index 1 --width 3840 --height 2160 --refresh 60
```

#### Output
```
Setting display mode...
Display mode set successfully.
Index: 0
Resolution: 2560x1440
Refresh Rate: 75Hz
```

---

### 9. `setlocation` - Set Display Location

Set position of virtual display in desktop topology.

#### Syntax
```bash
vddctl setlocation --index <index> --x <X coordinate> --y <Y coordinate> --width <width> --height <height>
```

#### Options
- `--index <n>`: Display index
- `--x <coordinate>`: X position
- `--y <coordinate>`: Y position
- `--width <width>`: Width
- `--height <height>`: Height

#### Examples
```batch
REM Place display to right of main display
vddctl setlocation --index 0 --x 1920 --y 0 --width 1920 --height 1080

REM Place display above
vddctl setlocation --index 0 --x 0 --y -1080 --width 1920 --height 1080

REM Multi-display layout
vddctl setlocation --index 1 --x 3840 --y 0 --width 2560 --height 1440
```

#### Output
```
Setting display location...
Display location set successfully.
Index: 0
Position: (1920, 0)
Size: 1920x1080
```

---

### 10. `setprimary` - Set Primary Display

Set virtual display as primary display.

#### Syntax
```bash
vddctl setprimary --index <index>
```

#### Options
- `--index <n>`: Index to set as primary display

#### Examples
```batch
REM Set first virtual display as primary
vddctl setprimary --index 0

REM Set second virtual display as primary
vddctl setprimary --index 1
```

#### Output
```
Setting primary display...
Primary display set successfully.
Index: 0
```

⚠️ **Note**: This will affect default display position for all applications

---

### 11. `list` - List Displays

List all displays and adapter information.

#### Syntax
```bash
vddctl list
```

#### Examples
```batch
vddctl list
```

#### Output
```
Listing displays and adapters...

Adapters:
  [0] Intel(R) UHD Graphics 630
  [1] IddSampleDriver Adapter

Displays:
  [0] Generic PnP Monitor (1920x1080 @ 60Hz) - Primary
  [1] Virtual Display (1920x1080 @ 60Hz)
  [2] Virtual Display (2560x1440 @ 75Hz)
```

---

### 12. `shutdown` - Shutdown SDK

Shutdown VDD SDK, release resources.

#### Syntax
```bash
vddctl shutdown
```

#### Examples
```batch
vddctl shutdown
```

#### Output
```
Shutting down VDD SDK...
VDD SDK shutdown successfully.
```

---

### 13. `help` - Display Help

Display command help information.

#### Syntax
```bash
vddctl help
```

#### Examples
```batch
vddctl help
```

---

## 📝 Complete Workflow Examples

### Example 1: Basic Installation and Activation

```batch
@echo off

REM 1. Initialize SDK
vddctl init

REM 2. Check status
vddctl status

REM 3. Install driver (requires administrator privileges)
vddctl install --inf IddSampleDriver_Fixed.inf

REM 4. Activate virtual display
vddctl activate --width 1920 --height 1080

REM 5. View display list
vddctl list

REM 6. Deactivate virtual display
vddctl deactivate

REM 7. Uninstall driver
vddctl uninstall
```

### Example 2: Create Multi-Display Layout

```batch
@echo off

REM Initialize
vddctl init
vddctl install --inf IddSampleDriver_Fixed.inf

REM Create two virtual displays
vddctl activate --width 1920 --height 1080 --count 2

REM Set first display (right of main display)
vddctl setlocation --index 0 --x 1920 --y 0 --width 1920 --height 1080

REM Set second display (right of first virtual display)
vddctl setlocation --index 1 --x 3840 --y 0 --width 1920 --height 1080

REM View layout
vddctl list
```

### Example 3: Test Different Resolutions

```batch
@echo off

vddctl init
vddctl install --inf IddSampleDriver_Fixed.inf

REM Test 1080p
vddctl activate --width 1920 --height 1080
timeout /t 5
vddctl deactivate

REM Test 1440p
vddctl activate --width 2560 --height 1440
timeout /t 5
vddctl deactivate

REM Test 4K
vddctl activate --width 3840 --height 2160
timeout /t 5
vddctl deactivate
```

---

## ⚠️ Important Notes

1. **Administrator Privileges**
   - `install` and `uninstall` commands require administrator privileges
   - Use `powershell Start-Process -Verb RunAs` to elevate privileges

2. **INF File**
   - Use `IddSampleDriver_Fixed.inf` (UTF-8 encoding, verified)
   - Works with `x64\Release\IddSampleDriver.dll`
   - No need to rebuild every time

3. **Driver Status**
   - Use `vddctl status` to check current status
   - Use `vddctl list` to view all displays

4. **Error Handling**
   - If command fails, check output error message
   - Use `--verbose` to get detailed logs

---

## 🔧 Troubleshooting

### Problem: "Driver Installed: No"

**Solution**:
```batch
REM Ensure using administrator privileges
powershell -Command "Start-Process 'vddctl.exe' -ArgumentList 'install --inf IddSampleDriver_Fixed.inf' -Verb RunAs -Wait"
```

### Problem: "Display Active: No"

**Reason**: Driver not installed or not initialized

**Solution**:
```batch
vddctl init
vddctl install --inf IddSampleDriver_Fixed.inf
vddctl activate --width 1920 --height 1080
```

### Problem: "Failed to enumerate adapters"

**Reason**: SDK not initialized

**Solution**:
```batch
vddctl init
```

---

## 📊 Command Quick Reference

| Command | Requires Admin | Main Purpose |
|---------|---------------|-------------|
| `init` | ❌ | Initialize SDK |
| `status` | ❌ | View status |
| `version` | ❌ | View version |
| `install` | ✅ | Install driver |
| `uninstall` | ✅ | Uninstall driver |
| `activate` | ❌ | Create virtual display |
| `deactivate` | ❌ | Remove virtual display |
| `setmode` | ❌ | Change resolution |
| `setlocation` | ❌ | Set position |
| `setprimary` | ❌ | Set as primary display |
| `list` | ❌ | List displays |
| `shutdown` | ❌ | Shutdown SDK |
| `help` | ❌ | Display help |
