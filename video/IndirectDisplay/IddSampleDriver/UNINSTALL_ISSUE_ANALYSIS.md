# 🔍 UninstallDriver Device Removal Issue Analysis

**Test Date:** 2025-11-01  
**Problem:** `vddctl uninstall` reports success, but devices still exist in Device Manager

---

## 📊 **Test Results**

### **Test 1: Modified Version**
```
vddctl uninstall
→ Output: "Driver uninstalled successfully"
→ Device Manager: 3 devices still present ❌
```

### **Test 2: Original Version (Git checkout)**
```
vddctl uninstall
→ Output: "Driver uninstalled successfully"  
→ Device Manager: 3 devices still present ❌
```

---

## 🎯 **Key Finding: Original Version Also Fails!**

**This indicates the problem is not in my modifications, but a deeper issue.**

---

## 🤔 **Possible Causes**

### **Cause 1: DIF_REMOVE call succeeds but device not actually removed**

```cpp
// UninstallDriver code (Line 767-782)
SP_REMOVEDEVICE_PARAMS removeParams = {};
removeParams.ClassInstallHeader.InstallFunction = DIF_REMOVE;
removeParams.Scope = DI_REMOVEDEVICE_GLOBAL;

if (SetupDiCallClassInstaller(DIF_REMOVE, hDevInfo, &devInfo)) {
    successCount++;  // ← Returns success here
} else {
    failCount++;
}
```

**Problem:** `SetupDiCallClassInstaller` returns TRUE, but device not actually removed.

**Possible reasons:**
- UMDF virtual display driver has special lifecycle management
- Need to stop a service first
- Driver locked by system or application

---

### **Cause 2: Virtual Display Driver Specifics**

**Virtual Display Driver (IDD) differs from regular PnP devices:**

1. **Software Device (ROOT enumerator)**
   - Not physical hardware
   - Windows may have special handling logic

2. **UMDF Driver**
   - Runs in user mode
   - May have host process (WUDFHost.exe) running

3. **Display System Integration**
   - Integrated with desktop manager
   - May require specific cleanup sequence

---

### **Cause 3: Additional Steps Required**

Potentially required steps:

#### **A. Stop Related Services First**
```batch
# Stop WUDF service (if applicable)
sc stop WUDFSvc
sc stop WUDFRd

# Then remove device
vddctl uninstall

# Restart service
sc start WUDFRd
```

#### **B. Remove Driver from Driver Store First**
```cpp
// Use DiUninstallDriverW
DiUninstallDriverW(NULL, infPath, 0, &reboot);

// Then remove device
SetupDiCallClassInstaller(DIF_REMOVE, ...);
```

#### **C. Use pnputil for removal**
```batch
# List drivers
pnputil /enum-drivers

# Remove driver package
pnputil /delete-driver oem##.inf /uninstall /force

# Then remove device node
vddctl uninstall
```

#### **D. Reboot Required**
```cpp
// Check if reboot needed
CM_Query_And_Remove_SubTree(..., CM_REMOVE_NO_RESTART, ...);
// May return reboot required
```

---

## 🔬 **Diagnostic Steps**

### **Step 1: Check `SetupDiCallClassInstaller` return value**

Modify `UninstallDriver` to add detailed logging:

```cpp
if (SetupDiCallClassInstaller(DIF_REMOVE, hDevInfo, &devInfo)) {
    successCount++;
    
    // Add: Check actual device status
    ULONG status, problem;
    DEVINST devInst;
    CM_Get_Device_Interface_Property(...);
    
} else {
    DWORD error = ::GetLastError();
    // Log error code
}
```

### **Step 2: Check if process is locking driver**

```powershell
# Check WUDFHost.exe process
Get-Process -Name WUDFHost -ErrorAction SilentlyContinue

# Check if driver file is locked
Get-Process | Where-Object {$_.Modules.FileName -like "*IddSampleDriver*"}
```

### **Step 3: Check device status**

```powershell
Get-PnpDevice -InstanceId "ROOT\IDDSAMPLEDRIVER\0000" | 
    Select-Object Status, Problem, ConfigManagerErrorCode
```

---

## 💡 **Proposed Solutions**

### **Solution A: Use PowerShell cmdlet (Most Reliable)** ⭐

```powershell
$devices = Get-PnpDevice -Class Display | 
    Where-Object { $_.InstanceId -like '*IddSampleDriver*' }

foreach ($dev in $devices) {
    # 1. Disable first
    Disable-PnpDevice -InstanceId $dev.InstanceId -Confirm:$false
    
    # 2. Then Remove
    Remove-PnpDevice -InstanceId $dev.InstanceId -Confirm:$false
}
```

**Advantages:**
- PowerShell cmdlets are Microsoft's recommended approach
- Automatically handles all complex situations
- Highest reliability

### **Solution B: Combined Method**

```cpp
// 1. Use pnputil to remove driver package first
system("pnputil /delete-driver oem##.inf /uninstall /force");

// 2. Stop related services
system("sc stop WUDFSvc");

// 3. Use SetupAPI to remove device
SetupDiCallClassInstaller(DIF_REMOVE, ...);

// 4. Restart service
system("sc start WUDFSvc");
```

### **Solution C: Mark as Reboot Required**

```cpp
// Use CM_Query_And_Remove_SubTree
DEVINST devInst;
CM_Locate_DevNodeW(&devInst, deviceId, 0);
CM_Query_And_Remove_SubTree(devInst, NULL, NULL, 
    CM_REMOVE_UI_OK,  // Allow UI
    CM_REMOVE_NO_RESTART);  // Mark but don't restart immediately

// Notify user reboot required
SetLastError("Device removal requires system restart");
```

---

## 🧪 **Next Test Steps**

### **Test A: PowerShell Manual Removal**

```batch
powershell -Command "$devices = Get-PnpDevice -Class Display | Where-Object { $_.InstanceId -like '*Idd*' }; foreach ($dev in $devices) { Disable-PnpDevice -InstanceId $dev.InstanceId -Confirm:$false; Remove-PnpDevice -InstanceId $dev.InstanceId -Confirm:$false }"
```

**If successful** → Using PowerShell cmdlets is the correct approach  
**If failed** → Need deeper diagnosis

### **Test B: Check if reboot required**

```batch
# Run vddctl uninstall
vddctl uninstall

# Reboot immediately
shutdown /r /t 0

# Check after reboot
Get-PnpDevice | Where-Object { $_.InstanceId -like '*Idd*' }
```

---

## 📝 **Summary**

1. ✅ **Confirmed problem not in modifications** (original also fails)
2. ❌ **SetupAPI DIF_REMOVE may not be sufficient for virtual display drivers**
3. 💡 **Need to use PowerShell cmdlets or combined method**
4. 🔧 **Next step: Test PowerShell Remove-PnpDevice**

---

## 🎯 **Recommended Action**

1. **Test immediately:** PowerShell manual removal
2. **If successful:** Modify `UninstallDriver` to call PowerShell cmdlets
3. **If failed:** Check if reboot required

Would you like me to test the PowerShell method immediately?
