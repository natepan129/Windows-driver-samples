# InstallDriver and UninstallDriver Implementation Review

## Context
This is a Windows Indirect Display Driver (IDD) installation/uninstallation implementation using SetupAPI. We need to verify the rollback logic and partial success handling are correct.

---

## InstallDriver Implementation

### Logic Flow:
1. **Validation** (Lines 586-595)
   - Check INF path not empty
   - Verify INF file exists
   
2. **Create Device Info List** (Lines 606-611)
   - `SetupDiCreateDeviceInfoList()` with Display Class GUID
   - **No rollback needed** - nothing created yet

3. **Create Device Info** (Lines 614-628)
   - `SetupDiCreateDeviceInfoW()` - creates device info structure (in memory only)
   - **No rollback needed** - device node not yet registered

4. **Set Hardware ID** (Lines 631-643)
   - `SetupDiSetDeviceRegistryPropertyW()` with `ROOT\IddSampleDriver`
   - **No rollback needed** - still in memory

5. **Register Device Node** (Lines 645-651) ⚠️ **CRITICAL POINT**
   - `SetupDiCallClassInstaller(DIF_REGISTERDEVICE)` 
   - **Device node NOW EXISTS in system**
   - **All failures after this point MUST rollback**

6. **Set Install Params** (Lines 654-669)
   - `SetupDiGetDeviceInstallParamsW()`
   - **On failure: ROLLBACK** - Remove device node via `DIF_REMOVE`

7. **Set INF Path** (Lines 671-688)
   - `SetupDiSetDeviceInstallParamsW()` with INF path
   - **On failure: ROLLBACK** - Remove device node

8. **Build Driver List** (Lines 690-705)
   - `SetupDiBuildDriverInfoList()` from INF
   - **On failure: ROLLBACK** - Remove device node

9. **Select Best Driver** (Lines 707-723)
   - `SetupDiCallClassInstaller(DIF_SELECTBESTCOMPATDRV)`
   - **On failure: ROLLBACK** - Remove device node

10. **Install Device** (Lines 725-759) ⚠️ **SPECIAL HANDLING**
    - `SetupDiCallClassInstaller(DIF_INSTALLDEVICE)`
    - **Check device status with `CM_Get_DevNode_Status()`**
    - **If API fails BUT device is functional (no ProblemCode, driver loaded):**
      - ✅ **Allow partial success** (e.g., signature warning)
    - **If API fails AND device has problems:**
      - ❌ **ROLLBACK** - Remove device node

### Key Question for Review:
**Is the partial success check correct?**
```cpp
if (!installSuccess) {
    if (cr == CR_SUCCESS && problemNumber == 0 && (status & DN_DRIVER_LOADED)) {
        // Allow partial success
    } else {
        // Rollback
    }
}
```

**Should we check for:**
- `DN_STARTED` in addition to `DN_DRIVER_LOADED`?
- Other status flags?
- Should we verify the device is in the "Display" class?

---

## UninstallDriver Implementation

### Logic Flow:
1. **Enumerate Display Devices** (Lines 775-815)
   - `SetupDiGetClassDevsW()` with Display Class GUID
   - Search for devices matching "IddSampleDriver" (case-insensitive)
   - Check both InstanceId and Description/FriendlyName
   - **Collect all matches into vector** (avoid iterator invalidation)

2. **Remove Each Device** (Lines 828-848)
   - For each matched device:
     - Set `SP_REMOVEDEVICE_PARAMS` with `DI_REMOVEDEVICE_GLOBAL`
     - Call `SetupDiCallClassInstaller(DIF_REMOVE)`
   - **Track success/failure counts**

3. **Return Partial Success** (Lines 853-861)
   - If some devices removed: Return `Status::Ok`
   - If all failed: Return `Status::DriverError`
   - Include counts in error message

### Key Questions for Review:
1. **Should UninstallDriver have rollback?**
   - Currently: No rollback if removal fails
   - Reasoning: Failed removal leaves device visible, user can retry
   - **Is this acceptable?**

2. **Should we verify device is not in use before removal?**
   - Check for active connections/monitors?
   - Force disable before removal?

3. **Scope: `DI_REMOVEDEVICE_GLOBAL` vs other scopes?**
   - Is `GLOBAL` correct for all scenarios?

---

## Specific Concerns:

### 1. Rollback Timing
- Device node is created at `DIF_REGISTERDEVICE` (Step 5)
- All failures after Step 5 call `DIF_REMOVE` to rollback
- **Is this the correct point to start rollback?**

### 2. Partial Success Detection
Current logic:
```cpp
// Allow partial success if:
cr == CR_SUCCESS &&           // Can query device
problemNumber == 0 &&         // No problem code
(status & DN_DRIVER_LOADED)   // Driver loaded
```

**Alternative checks?**
- `DN_STARTED` - device is started
- `DN_DISABLEABLE` - can be disabled
- Verify device has correct Class GUID?

### 3. Orphaned Device Scenarios
**When can orphaned devices occur?**
- Install fails at Steps 5-7 but rollback fails
- System crashes during installation
- Manual termination during install

**Should we add a cleanup function?**
- Find devices with no driver assigned
- Remove orphaned `ROOT\IDDSAMPLEDRIVER` devices

### 4. Uninstall Safety
**Current behavior:**
- No pre-checks before removal
- No verification device is inactive
- Partial success returns OK

**Potential issues:**
- Removing active display adapter?
- Black screen if primary display?

---

## Questions for AI Review:

1. **Is the rollback logic correct and complete?**
2. **Is the partial success check sufficient? Should we add more status checks?**
3. **Should UninstallDriver verify device is inactive before removal?**
4. **Are there any race conditions or concurrency issues?**
5. **Should we add orphaned device cleanup?**
6. **Is `DI_REMOVEDEVICE_GLOBAL` the correct scope for uninstall?**
7. **Any missing error handling or edge cases?**

---

## API References:
- `SetupDiCreateDeviceInfoList` - Creates device information set
- `SetupDiCreateDeviceInfoW` - Adds device to list (memory only)
- `SetupDiSetDeviceRegistryPropertyW` - Sets properties (memory only)
- `DIF_REGISTERDEVICE` - **Registers device node in system** ⚠️
- `SetupDiSetDeviceInstallParamsW` - Sets INF path
- `SetupDiBuildDriverInfoList` - Builds driver list from INF
- `DIF_SELECTBESTCOMPATDRV` - Selects best matching driver
- `DIF_INSTALLDEVICE` - Installs driver to device
- `CM_Get_DevNode_Status` - Gets device status and problem code
- `DIF_REMOVE` - Removes device from system
- `DN_DRIVER_LOADED` - Driver loaded flag
- `DN_STARTED` - Device started flag


