# Design: Correct Display Configuration Patterns

## Context

v1.0.0 display configuration has critical flaws in **five areas**:

**SDK Layer (vddsdk.cpp)**:
1. **SetDisplayConfig misuse**: Blind query-and-replay causes topology corruption
2. **Device identification**: String-based matching fails across languages/OEMs
3. **DEVMODE handling**: Incomplete structure preservation causes rejection

**Driver Layer (Driver.cpp)**:
4. **ContainerId instability**: Runtime GUID generation causes layout resets
5. **EDID corruption**: Incorrect checksums cause OS to ignore EDID data

This document provides **correct patterns** for each operation across both layers.

---

## Goals

- **Zero black screens** on physical multi-monitor setups
- **Reliable device targeting** across all Windows languages
- **Functional SetPrimary()** that actually changes primary display
- **Robust mode setting** that respects all DEVMODE fields

## Non-Goals

- Cross-platform support (Windows-only by design)
- Support for Windows 7/8 (requires Windows 10 2004+)
- Hot-plug event handling (defer to v2.0)

---

## Core Patterns

### Pattern 1: Device Identification (Hardware ID-Based)

❌ **WRONG** (String matching - fails on localized systems):

```cpp
// BAD: Relies on DeviceString, breaks on non-English Windows
if (deviceString.find(L"IddSampleDriver") != std::wstring::npos) {
    // Fragile!
}

// BAD: Hardcoded EDID fragments, OEM-specific
if (monitorId.find(L"DELD0E6") != std::wstring::npos) {
    // Will miss other EDIDs
}
```

✅ **CORRECT** (Hardware ID matching - language-independent):

```cpp
// GOOD: Use SetupAPI to enumerate by Hardware ID
std::vector<std::wstring> GetVirtualDisplayDeviceNames() {
    std::vector<std::wstring> deviceNames;
    
    // Step 1: Find device instances with our HWID
    HDEVINFO hDevInfo = SetupDiGetClassDevsW(
        &GUID_DEVCLASS_DISPLAY,
        L"ROOT\\IddSampleDriver",  // Our Hardware ID
        nullptr,
        DIGCF_PRESENT
    );
    
    if (hDevInfo == INVALID_HANDLE_VALUE) return deviceNames;
    
    // Step 2: Get device instance IDs
    std::vector<DEVINST> deviceInstances;
    SP_DEVINFO_DATA devInfo = { sizeof(SP_DEVINFO_DATA) };
    for (DWORD i = 0; SetupDiEnumDeviceInfo(hDevInfo, i, &devInfo); i++) {
        deviceInstances.push_back(devInfo.DevInst);
    }
    SetupDiDestroyDeviceInfoList(hDevInfo);
    
    // Step 3: Map device instances to GDI device names via DisplayConfig
    UINT32 numPaths, numModes;
    GetDisplayConfigBufferSizes(QDC_ALL_PATHS, &numPaths, &numModes);
    
    std::vector<DISPLAYCONFIG_PATH_INFO> paths(numPaths);
    std::vector<DISPLAYCONFIG_MODE_INFO> modes(numModes);
    QueryDisplayConfig(QDC_ALL_PATHS, &numPaths, paths.data(), &numModes, modes.data(), nullptr);
    
    // Step 4: Match adapterId from DisplayConfig to our device instances
    for (const auto& path : paths) {
        DISPLAYCONFIG_SOURCE_DEVICE_NAME sourceName = {};
        sourceName.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_SOURCE_NAME;
        sourceName.header.size = sizeof(sourceName);
        sourceName.header.adapterId = path.sourceInfo.adapterId;
        sourceName.header.id = path.sourceInfo.id;
        
        if (DisplayConfigGetDeviceInfo(&sourceName.header) == ERROR_SUCCESS) {
            // TODO: Verify this adapterId belongs to our device instance
            // For now, use naming convention or store mapping during install
            deviceNames.push_back(sourceName.viewGdiDeviceName);
        }
    }
    
    return deviceNames;
}
```

**Improvement for v1.1**: Store adapterId + sourceId during installation for direct lookup.

---

### Pattern 2: SetPrimary() (Correct Way)

❌ **WRONG** (Only changes position, doesn't set primary flag):

```cpp
// BAD: This just moves the display to (0,0)
sourceMode.position.x = 0;
sourceMode.position.y = 0;
SetDisplayConfig(...);  // Windows doesn't interpret this as "set primary"
```

✅ **CORRECT** (Use CDS_SET_PRIMARY flag):

```cpp
Status SetPrimary(uint32_t outputIndex) {
    // Get target device name
    auto devices = GetVirtualDisplayDeviceNames();
    if (outputIndex >= devices.size()) return Status::InvalidArg;
    
    std::wstring deviceName = devices[outputIndex];
    
    // Method 1: Simple and reliable
    LONG result = ChangeDisplaySettingsExW(
        deviceName.c_str(),
        nullptr,                // NULL = use current settings
        nullptr,
        CDS_SET_PRIMARY | CDS_UPDATEREGISTRY | CDS_NORESET,
        nullptr
    );
    
    if (result != DISP_CHANGE_SUCCESSFUL) {
        SetLastError("Failed to set primary display");
        return Status::DriverError;
    }
    
    // Commit all pending changes
    ChangeDisplaySettingsExW(nullptr, nullptr, nullptr, 0, nullptr);
    
    // Verify primary was set
    DEVMODEW dm = { sizeof(dm) };
    if (EnumDisplaySettingsW(deviceName.c_str(), ENUM_CURRENT_SETTINGS, &dm)) {
        if (dm.dmPosition.x == 0 && dm.dmPosition.y == 0) {
            // Primary displays are at origin
            printf("[VDD] Primary display set successfully\n");
            return Status::Ok;
        }
    }
    
    return Status::DriverError;
}
```

---

### Pattern 3: SetMode() (Preserve DEVMODE Structure)

❌ **WRONG** (Only sets width/height/refresh, breaks on rotation/bpp):

```cpp
// BAD: Missing fields cause ERROR_INVALID_PARAMETER
DEVMODEW dm = {};
dm.dmSize = sizeof(dm);
dm.dmPelsWidth = 1920;
dm.dmPelsHeight = 1080;
dm.dmDisplayFrequency = 60;
dm.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_DISPLAYFREQUENCY;
ChangeDisplaySettingsExW(device, &dm, ...);  // Often fails!
```

✅ **CORRECT** (Query current, modify target fields):

```cpp
Status SetMode(uint32_t outputIndex, const DisplayMode& mode) {
    auto devices = GetVirtualDisplayDeviceNames();
    if (outputIndex >= devices.size()) return Status::InvalidArg;
    
    std::wstring deviceName = devices[outputIndex];
    
    // Step 1: Get current settings as baseline
    DEVMODEW dm = { sizeof(dm) };
    if (!EnumDisplaySettingsW(deviceName.c_str(), ENUM_CURRENT_SETTINGS, &dm)) {
        return Status::DriverError;
    }
    
    // Step 2: Modify only target fields
    dm.dmPelsWidth = mode.width;
    dm.dmPelsHeight = mode.height;
    
    // Optional: Set refresh rate (be careful with 59.94 vs 60)
    if (mode.refreshNumerator > 0 && mode.refreshDenominator > 0) {
        DWORD refreshHz = mode.refreshNumerator / mode.refreshDenominator;
        dm.dmDisplayFrequency = refreshHz;
        dm.dmFields |= DM_DISPLAYFREQUENCY;
    } else {
        // Let Windows pick best refresh rate
        dm.dmFields &= ~DM_DISPLAYFREQUENCY;
    }
    
    // Preserve: dmBitsPerPel, dmDisplayOrientation, dmDisplayFlags, dmDisplayFixedOutput
    // These are already set from ENUM_CURRENT_SETTINGS
    
    // Step 3: Validate mode is supported
    bool supported = false;
    DEVMODEW testDm = { sizeof(testDm) };
    for (DWORD i = 0; EnumDisplaySettingsW(deviceName.c_str(), i, &testDm); i++) {
        if (testDm.dmPelsWidth == dm.dmPelsWidth &&
            testDm.dmPelsHeight == dm.dmPelsHeight) {
            supported = true;
            break;
        }
    }
    
    if (!supported) {
        SetLastError("Requested mode not supported by display");
        return Status::InvalidArg;
    }
    
    // Step 4: Apply
    LONG result = ChangeDisplaySettingsExW(
        deviceName.c_str(),
        &dm,
        nullptr,
        CDS_UPDATEREGISTRY | CDS_NORESET,
        nullptr
    );
    
    if (result == DISP_CHANGE_SUCCESSFUL) {
        // Commit
        ChangeDisplaySettingsExW(nullptr, nullptr, nullptr, 0, nullptr);
        return Status::Ok;
    } else if (result == DISP_CHANGE_BADMODE) {
        SetLastError("Invalid display mode");
        return Status::InvalidArg;
    } else {
        SetLastError("Failed to change display mode");
        return Status::DriverError;
    }
}
```

---

### Pattern 4: SetDisplayConfig (Correct Topology Strategy)

❌ **WRONG** (Blind query-and-replay corrupts topology):

```cpp
// BAD: This pattern is dangerous
QueryDisplayConfig(QDC_ONLY_ACTIVE_PATHS, ...);
// ... blindly replay ...
SetDisplayConfig(..., SDC_APPLY | SDC_USE_SUPPLIED_DISPLAY_CONFIG | SDC_SAVE_TO_DATABASE);
// Windows may rearrange displays, causing black screen!
```

✅ **CORRECT** (Explicit topology strategy):

```cpp
Status Activate(...) {
    // Enable device first (via SetupDiCallClassInstaller)
    // ...
    
    // Don't blindly replay! Use explicit topology strategy instead
    
    // Option A: Simple extend (safest)
    LONG result = SetDisplayConfig(
        0, nullptr,  // numPaths=0, paths=nullptr means "use default"
        0, nullptr,  // numModes=0, modes=nullptr
        SDC_TOPOLOGY_EXTEND | SDC_APPLY | SDC_SAVE_TO_DATABASE
    );
    
    if (result == ERROR_SUCCESS) {
        printf("[VDD] Extended desktop to include virtual display\n");
        return Status::Ok;
    }
    
    // Option B: Explicit path modification (for SetLocation)
    // Only use SDC_USE_SUPPLIED_DISPLAY_CONFIG when you've carefully
    // constructed the path/mode arrays
    
    return Status::Ok;
}

Status SetLocation(uint32_t outputIndex, const DisplayRect& rect) {
    // Step 1: Query current configuration
    UINT32 numPaths, numModes;
    GetDisplayConfigBufferSizes(QDC_ONLY_ACTIVE_PATHS, &numPaths, &numModes);
    
    std::vector<DISPLAYCONFIG_PATH_INFO> paths(numPaths);
    std::vector<DISPLAYCONFIG_MODE_INFO> modes(numModes);
    QueryDisplayConfig(QDC_ONLY_ACTIVE_PATHS, &numPaths, paths.data(), &numModes, modes.data(), nullptr);
    
    // Step 2: Find target path (by device name matching)
    auto devices = GetVirtualDisplayDeviceNames();
    std::wstring targetDevice = devices[outputIndex];
    
    int targetPathIndex = -1;
    for (UINT32 i = 0; i < numPaths; i++) {
        DISPLAYCONFIG_SOURCE_DEVICE_NAME sourceName = {};
        sourceName.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_SOURCE_NAME;
        sourceName.header.size = sizeof(sourceName);
        sourceName.header.adapterId = paths[i].sourceInfo.adapterId;
        sourceName.header.id = paths[i].sourceInfo.id;
        
        if (DisplayConfigGetDeviceInfo(&sourceName.header) == ERROR_SUCCESS) {
            if (wcscmp(sourceName.viewGdiDeviceName, targetDevice.c_str()) == 0) {
                targetPathIndex = i;
                break;
            }
        }
    }
    
    if (targetPathIndex < 0) {
        return Status::NotFound;
    }
    
    // Step 3: Modify ONLY the target source's position
    UINT32 modeIdx = paths[targetPathIndex].sourceInfo.modeInfoIdx;
    if (modeIdx >= numModes || 
        modes[modeIdx].infoType != DISPLAYCONFIG_MODE_INFO_TYPE_SOURCE) {
        return Status::DriverError;
    }
    
    modes[modeIdx].sourceMode.position.x = rect.x;
    modes[modeIdx].sourceMode.position.y = rect.y;
    // width/height are already set correctly
    
    // CRITICAL: Keep path.flags unchanged (especially DISPLAYCONFIG_PATH_ACTIVE)
    // Don't modify other paths!
    
    // Step 4: Validate before applying
    LONG result = SetDisplayConfig(
        numPaths, paths.data(),
        numModes, modes.data(),
        SDC_VALIDATE | SDC_USE_SUPPLIED_DISPLAY_CONFIG
    );
    
    if (result != ERROR_SUCCESS) {
        SetLastError("Display configuration validation failed");
        return Status::DriverError;
    }
    
    // Step 5: Apply with ALLOW_CHANGES for flexibility
    result = SetDisplayConfig(
        numPaths, paths.data(),
        numModes, modes.data(),
        SDC_APPLY | SDC_USE_SUPPLIED_DISPLAY_CONFIG | SDC_SAVE_TO_DATABASE | SDC_ALLOW_CHANGES
    );
    
    if (result == ERROR_SUCCESS) {
        return Status::Ok;
    } else {
        SetLastError("Failed to set display location");
        return Status::DriverError;
    }
}
```

**Key Points**:
- Use `SDC_TOPOLOGY_EXTEND` when you just want to add displays
- Only use `SDC_USE_SUPPLIED_DISPLAY_CONFIG` when you've validated the arrays
- Always call `SDC_VALIDATE` first
- Add `SDC_ALLOW_CHANGES` for flexibility
- Never modify paths you don't understand

---

## Architectural Decisions

### Decision 1: Dual Strategy (SetDisplayConfig + ChangeDisplaySettingsExW)

**Choice**: Keep both methods, use SetDisplayConfig primarily with ChangeDisplaySettingsExW fallback

**Rationale**:
- SetDisplayConfig: Modern, supports multi-monitor topology control
- ChangeDisplaySettingsExW: Legacy, simpler, better compatibility

**Implementation**:
```cpp
// Try modern API first
if (SetDisplayConfigApproach() == Status::Ok) {
    return Status::Ok;
}

// Fall back to legacy
printf("[VDD] SetDisplayConfig failed, trying ChangeDisplaySettingsExW...\n");
return ChangeDisplaySettingsExWApproach();
```

---

### Decision 2: Hardware ID vs Device Interface

**Choice**: Use Hardware ID (ROOT\\IddSampleDriver) for device identification

**Alternatives**:
- Device String: Too fragile, language-dependent
- EDID fragments: OEM-specific, doesn't scale
- Device Interface GUID: More robust, but requires GUID registration

**Trade-offs**:
- HWID is simple and works for our use case
- For v2.0, consider registering a device interface GUID for more robust identification

---

### Decision 3: When to Use SetDisplayConfig

**Use SetDisplayConfig for**:
- Setting display location (topology control)
- Complex multi-monitor arrangements
- When you need to preserve exact path/mode configuration

**Use ChangeDisplaySettingsExW for**:
- Setting primary display (CDS_SET_PRIMARY)
- Simple mode changes (resolution only)
- When SetDisplayConfig is too complex

**Don't use SetDisplayConfig for**:
- Blind query-and-replay (dangerous!)
- Simple operations better served by ChangeDisplaySettingsExW

---

## Implementation Notes

### SPDRP_HARDWAREID Multi-String Iteration

```cpp
WCHAR hwid[1024] = {};
if (SetupDiGetDeviceRegistryPropertyW(hDevInfo, &devInfo,
    SPDRP_HARDWAREID, nullptr, (BYTE*)hwid, sizeof(hwid), nullptr)) {
    
    // Iterate through MULTI_SZ (double-null terminated)
    for (wchar_t* ptr = hwid; *ptr; ptr += wcslen(ptr) + 1) {
        if (_wcsicmp(ptr, L"ROOT\\IddSampleDriver") == 0) {
            // Found it!
            break;
        }
    }
}
```

### Activation Stabilization (Polling vs Fixed Delay)

```cpp
bool WaitForDeviceReady(DEVINST devInst, DWORD timeoutMs) {
    DWORD elapsed = 0;
    while (elapsed < timeoutMs) {
        ULONG status = 0, problem = 0;
        if (CM_Get_DevNode_Status(&status, &problem, devInst, 0) == CR_SUCCESS) {
            if ((status & DN_STARTED) && !(status & DN_HAS_PROBLEM)) {
                return true;  // Device ready
            }
        }
        Sleep(100);
        elapsed += 100;
    }
    return false;  // Timeout
}
```

### Refresh Rate Handling (Fractional Hz)

```cpp
// Don't round 59.94 to 59!
// Option 1: Use rational (numerator/denominator)
struct DisplayMode {
    uint32_t width, height;
    uint32_t refreshNumerator;    // e.g., 60000
    uint32_t refreshDenominator;  // e.g., 1001 (=59.94Hz)
};

// Option 2: Don't specify refresh rate, let Windows choose
dm.dmFields &= ~DM_DISPLAYFREQUENCY;  // Don't set this field
```

---

## Driver Layer Patterns

### Pattern 7: Stable ContainerId Generation

❌ **WRONG** (Runtime GUID - resets layout every reboot):

```cpp
// BAD: New GUID every time FinishInit() runs
HRESULT hr = CoCreateGuid(&MonitorContainerId);
// Windows sees this as a "new monitor" → desktop layout reset
```

✅ **CORRECT** (Deterministic GUID - stable across reboots):

```cpp
// Generate stable GUID from fixed namespace + ConnectorIndex
GUID GenerateStableContainerId(UINT ConnectorIndex) {
    // Use your vendor namespace GUID
    static const GUID NAMESPACE_GUID = {
        0x12345678, 0x1234, 0x5678, 
        {0x12, 0x34, 0x56, 0x78, 0x90, 0xAB, 0xCD, 0xEF}
    };
    
    // Seed = Namespace + Index
    BYTE seed[sizeof(GUID) + sizeof(UINT)];
    memcpy(seed, &NAMESPACE_GUID, sizeof(GUID));
    memcpy(seed + sizeof(GUID), &ConnectorIndex, sizeof(UINT));
    
    // Hash to GUID (use RtlComputeCrc32 or similar)
    GUID result;
    // ... deterministic hash algorithm ...
    return result;
}

// In FinishInit():
MonitorContainerId = GenerateStableContainerId(pContext->ConnectorIndex);
DbgPrint("[IddSample] Monitor %d: Stable ContainerId = {%08X-...}\n", 
         pContext->ConnectorIndex, MonitorContainerId.Data1);
```

**Why this matters**:
- Windows uses ContainerId to identify monitors across sessions
- Stable GUID → Windows remembers display position, color profiles, scaling
- Unstable GUID → Layout reset = terrible UX

**Implementation notes**:
- Use same namespace GUID for all 3 monitors
- ConnectorIndex differentiates them
- Optional: Persist to registry as backup

---

### Pattern 8: EDID Checksum Validation

❌ **WRONG** (Invalid checksum - OS ignores EDID):

```cpp
// BAD: Checksum not validated
static const BYTE edid_block[128] = {
    0x00, 0xFF, 0xFF, ...,
    0xDF  // <-- WRONG checksum, should be 0x0A
};
// Result: Windows falls back to generic 640x480, 800x600, 1024x768
```

✅ **CORRECT** (Validated checksum):

```cpp
// Calculate correct EDID checksum
BYTE CalculateEdidChecksum(const BYTE* edid, size_t len) {
    UINT sum = 0;
    for (size_t i = 0; i < len - 1; i++) {
        sum += edid[i];
    }
    return (BYTE)((256 - (sum % 256)) % 256);
}

// Validate at compile time
static const BYTE s_MonitorEdid[128] = {
    // ... EDID data ...
    0x0A  // Correct checksum
};

#ifdef _DEBUG
static_assert(
    CalculateEdidChecksum(s_MonitorEdid, 128) == s_MonitorEdid[127],
    "EDID checksum mismatch!"
);
#endif

// At runtime validation:
bool ValidateEdidChecksum(const BYTE* edid, size_t len) {
    UINT sum = 0;
    for (size_t i = 0; i < len; i++) {
        sum += edid[i];
    }
    return (sum % 256) == 0;  // Valid if sum divisible by 256
}
```

**Checksum formula** (EDID 1.3 standard):
1. Sum bytes[0..126]
2. Checksum = (256 - (sum % 256)) % 256
3. Byte[127] = checksum
4. Verification: (sum of all 128 bytes) % 256 == 0

**Known issues in sample code**:
- Monitor 0 (Dell): Checksum likely correct
- Monitor 1 (Lenovo): May need correction
- Monitor 2 (HP): Checksum was 0xDF, corrected to 0x0A

**Testing**:
```cpp
// Add to driver init:
for (UINT i = 0; i < ARRAYSIZE(s_SampleMonitors); i++) {
    bool valid = ValidateEdidChecksum(
        s_SampleMonitors[i].pEdidBlock, 
        IndirectSampleMonitor::szEdidBlock
    );
    DbgPrint("[IddSample] Monitor %d EDID checksum: %s\n", 
             i, valid ? "VALID" : "INVALID");
}
```

---

## Testing Strategy

### Black Screen Prevention Tests

**Test 1: Rapid Activate/Deactivate**
- 100 cycles on physical machine
- Verify no black screen, no hung display

**Test 2: Multi-Monitor Stress**
- Laptop + 2 external monitors
- Activate VDD, verify all 3 existing displays still work

**Test 3: Primary Display Swap**
- SetPrimary to VDD
- Verify taskbar moves
- SetPrimary back to physical
- Verify taskbar returns

### Device Identification Tests

**Test 1: Language Independence**
- Test on English, Chinese, Japanese Windows
- Verify device found in all cases

**Test 2: Multi-HWID Device**
- Mock device with 3+ compatible IDs
- Verify detection works regardless of order

### Mode Setting Tests

**Test 1: Common Resolutions**
- 1920x1080, 2560x1440, 3840x2160
- Verify all apply successfully

**Test 2: Fractional Refresh Rates**
- 59.94Hz panel
- Set 60Hz mode
- Verify no ERROR_INVALID_PARAMETER

---

## Performance Targets

| Operation | Target | Notes |
|-----------|--------|-------|
| Device identification | < 100ms | Cached after first query |
| SetPrimary | < 300ms | Includes taskbar migration |
| SetMode | < 1s | Includes display mode switch |
| SetLocation | < 500ms | Topology recalculation |
| Activate with polling | < 3s | Device stabilization |

---

## References

- **SetDisplayConfig**: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setdisplayconfig
- **ChangeDisplaySettingsExW**: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-changedisplaysettingsexw
- **SetupAPI**: https://learn.microsoft.com/en-us/windows-hardware/drivers/install/setupapi
- **DisplayConfig Structures**: https://learn.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-displayconfig_path_info

