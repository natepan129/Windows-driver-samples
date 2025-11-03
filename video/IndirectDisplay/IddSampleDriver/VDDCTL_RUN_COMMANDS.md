# How to Run vddctl Commands

## ❌ Common Errors

### Path Issues in PowerShell

**Wrong Command** (PowerShell):
```powershell
build\bin\Release\vddctl.exe init
# Error: The module 'build' could not be loaded
```

---

## ✅ Correct Running Methods

### Method 1: Use `.\` Prefix in PowerShell

```powershell
.\build\bin\Release\vddctl.exe init
.\build\bin\Release\vddctl.exe status
.\build\bin\Release\vddctl.exe install --inf IddSampleDriver_Fixed.inf
```

### Method 2: Use CMD (Command Prompt)

```batch
build\bin\Release\vddctl.exe init
build\bin\Release\vddctl.exe status
build\bin\Release\vddctl.exe install --inf IddSampleDriver_Fixed.inf
```

### Method 3: Use Full Path

```powershell
# PowerShell
C:\Users\...\IddSampleDriver\build\bin\Release\vddctl.exe init

# CMD
C:\Users\...\IddSampleDriver\build\bin\Release\vddctl.exe init
```

### Method 4: CD to Directory First

```batch
cd build\bin\Release
vddctl.exe init
vddctl.exe status
```

### Method 5: Use & Operator (PowerShell)

```powershell
& "build\bin\Release\vddctl.exe" init
& "build\bin\Release\vddctl.exe" status
```

---

## 🚀 Recommended Method

### Run in CMD (Simplest)

```batch
REM Open CMD (not PowerShell)
REM Enter project directory
cd C:\Users\WDKRemoteUser.WIN10TESTING.000\Desktop\Windows-driver-samples\video\IndirectDisplay\IddSampleDriver

REM Run commands directly
build\bin\Release\vddctl.exe init
build\bin\Release\vddctl.exe install --inf IddSampleDriver_Fixed.inf
build\bin\Release\vddctl.exe status
```

### Use Provided Batch Scripts

```batch
REM Test initialization
cmd /c test_vddctl_init.bat

REM Install driver
cmd /c install_with_vddctl.bat

REM Complete test
cmd /c test_vddctl_final.bat
```

---

## 📋 Quick Reference

| Environment | Command Format | Example |
|------|---------|------|
| CMD | `path\vddctl.exe command` | `build\bin\Release\vddctl.exe init` |
| PowerShell | `.\path\vddctl.exe command` | `.\build\bin\Release\vddctl.exe init` |
| PowerShell (& operator) | `& "path\vddctl.exe" command` | `& "build\bin\Release\vddctl.exe" init` |
| Full path | `C:\fullpath\vddctl.exe command` | `C:\...\vddctl.exe init` |

---

## 🎯 Ready-to-Use Commands (Copy & Paste)

### If You're in PowerShell:

```powershell
# Initialize
.\build\bin\Release\vddctl.exe init

# View status
.\build\bin\Release\vddctl.exe status

# View version
.\build\bin\Release\vddctl.exe version

# Install driver (requires administrator)
Start-Process ".\build\bin\Release\vddctl.exe" -ArgumentList "install --inf IddSampleDriver_Fixed.inf" -Verb RunAs -Wait

# Activate display
.\build\bin\Release\vddctl.exe activate --width 1920 --height 1080

# List displays
.\build\bin\Release\vddctl.exe list
```

### If You're in CMD:

```batch
REM Initialize
build\bin\Release\vddctl.exe init

REM View status
build\bin\Release\vddctl.exe status

REM View version
build\bin\Release\vddctl.exe version

REM Install driver (will show UAC)
build\bin\Release\vddctl.exe install --inf IddSampleDriver_Fixed.inf

REM Activate display
build\bin\Release\vddctl.exe activate --width 1920 --height 1080

REM List displays
build\bin\Release\vddctl.exe list
```

---

## 💡 Simplest Method

**Create a shortcut script** `vdd.bat`:

```batch
@echo off
build\bin\Release\vddctl.exe %*
```

Then you can simply run:

```batch
vdd init
vdd status
vdd install --inf IddSampleDriver_Fixed.inf
vdd activate --width 1920 --height 1080
vdd list
```
