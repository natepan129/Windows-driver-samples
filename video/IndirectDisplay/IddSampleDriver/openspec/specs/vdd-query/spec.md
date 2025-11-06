# VDD Query Capability

## Purpose

This capability provides functions to enumerate and query display adapters, monitors, supported modes, and DXGI outputs for application targeting.

---

## Requirements

### Requirement: Display Adapter Enumeration

The system SHALL provide a function to list all display adapters (physical and virtual) in the system.

#### Scenario: Enumerate all adapters
- **WHEN** `EnumerateAdapters()` is called
- **THEN** the function returns a vector of `AdapterInfo` structures
- **AND** each entry includes device instance ID, virtual flag, active status, and friendly name
- **AND** virtual display adapters are marked with `isVirtual = true`
- **AND** physical adapters (e.g., NVIDIA, Intel, AMD) are marked with `isVirtual = false`

#### Scenario: Identify VDD adapters
- **WHEN** `EnumerateAdapters()` returns results
- **THEN** VDD adapters have names containing "IddSampleDriver" or matching EDID patterns
- **AND** can be distinguished from physical adapters programmatically

#### Scenario: Check adapter active status
- **WHEN** `EnumerateAdapters()` is called after `Activate()`
- **THEN** VDD adapters show `isActive = true`
- **AND** after `Deactivate()`, VDD adapters show `isActive = false`

---

### Requirement: Display Mode Enumeration

The system SHALL provide a function to list all supported display modes for a given output.

#### Scenario: Enumerate modes for virtual display
- **WHEN** `EnumerateModes()` is called with a virtual display output index
- **THEN** the function returns modes advertised in the driver's EDID
- **AND** modes include resolution (width × height) and refresh rate
- **AND** modes are sorted by resolution (highest first) and refresh rate

#### Scenario: Enumerate standard modes
- **WHEN** `EnumerateModes()` is called for a VDD with standard EDID
- **THEN** common modes are available: 1920×1080, 2560×1440, 3840×2160
- **AND** refresh rates include 60Hz, 90Hz, 120Hz variants

#### Scenario: Query modes for inactive display
- **WHEN** `EnumerateModes()` is called for an inactive virtual display
- **THEN** the function returns `Status::NotActive`
- **AND** provides guidance to activate the display first

---

### Requirement: DXGI Output Discovery

The system SHALL provide a function to find a specific DXGI output by name for fullscreen application targeting.

#### Scenario: Find DXGI output for VDD
- **WHEN** `FindDxgiOutputByName()` is called with "VDD XR"
- **THEN** the function enumerates DXGI adapters and outputs
- **AND** returns the `IDXGIOutput` pointer matching the display name
- **AND** the output can be used for DXGI fullscreen exclusive mode

#### Scenario: Find output that doesn't exist
- **WHEN** `FindDxgiOutputByName()` is called with a non-existent name
- **THEN** the function returns `Status::NotFound`
- **AND** the output pointer is set to `nullptr`

#### Scenario: DXGI output for primary display
- **WHEN** `FindDxgiOutputByName()` is called for the primary display
- **THEN** the function correctly identifies the primary output
- **AND** the DXGI adapter LUID matches the display adapter

---

### Requirement: System Information Query

The system SHALL provide a function to retrieve system information relevant to VDD operations.

#### Scenario: Query system info
- **WHEN** `GetSystemInfo()` is called
- **THEN** the function returns Windows version, build number, and architecture
- **AND** total/available system memory is reported
- **AND** number of logical processors is reported
- **AND** display configuration (number of active adapters) is included

#### Scenario: Validate OS compatibility
- **WHEN** SDK operations are performed on Windows 10 2004+
- **THEN** all DisplayConfig APIs are available
- **AND** WDDM 2.x features are supported

---

### Requirement: SDK Version Query

The system SHALL provide a function to retrieve the SDK version.

#### Scenario: Query SDK version
- **WHEN** `GetVersion()` is called
- **THEN** the function returns a `Version` struct with major, minor, patch
- **AND** the version follows semantic versioning rules

---

## Implementation Notes

### Adapter Enumeration Implementation

Uses Windows `EnumDisplayDevices` API in two passes:

**Pass 1: Enumerate adapters**
```cpp
DISPLAY_DEVICEW adapter = {};
adapter.cb = sizeof(DISPLAY_DEVICEW);
for (DWORD i = 0; EnumDisplayDevicesW(nullptr, i, &adapter, 0); i++) {
    // Check adapter.DeviceString for "IddSampleDriver"
}
```

**Pass 2: Enumerate monitors per adapter**
```cpp
DISPLAY_DEVICEW monitor = {};
monitor.cb = sizeof(DISPLAY_DEVICEW);
for (DWORD j = 0; EnumDisplayDevicesW(adapter.DeviceName, j, &monitor, 0); j++) {
    // Check monitor.DeviceID for EDID patterns (DELD0E6, LEN65BF, etc.)
}
```

### Mode Enumeration Implementation

Uses `EnumDisplaySettingsW` to query supported modes:

```cpp
DEVMODEW devMode = {};
devMode.dmSize = sizeof(DEVMODEW);
for (DWORD i = 0; EnumDisplaySettingsW(deviceName, i, &devMode); i++) {
    DisplayMode mode = {
        .width = devMode.dmPelsWidth,
        .height = devMode.dmPelsHeight,
        .refreshNumerator = devMode.dmDisplayFrequency,
        .refreshDenominator = 1
    };
    // Add to results
}
```

### DXGI Output Discovery Implementation

Enumerates DXGI factory, adapters, and outputs:

```cpp
IDXGIFactory* factory;
CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory);

for (UINT adapterIdx = 0; factory->EnumAdapters(adapterIdx, &adapter) != DXGI_ERROR_NOT_FOUND; ++adapterIdx) {
    for (UINT outputIdx = 0; adapter->EnumOutputs(outputIdx, &output) != DXGI_ERROR_NOT_FOUND; ++outputIdx) {
        DXGI_OUTPUT_DESC desc;
        output->GetDesc(&desc);
        // Match desc.DeviceName against target name
    }
}
```

### System Information Sources

| Information | Source API | Notes |
|-------------|------------|-------|
| OS Version | `RtlGetVersion` (ntdll.dll) | More reliable than `GetVersionEx` |
| Memory | `GlobalMemoryStatusEx` | Total/available physical memory |
| CPU Count | `GetSystemInfo` | Logical processor count |
| Architecture | `GetNativeSystemInfo` | x64 vs x86 detection |

---

## Performance Characteristics

| Operation | Typical Duration | Notes |
|-----------|------------------|-------|
| EnumerateAdapters | 50-200ms | Depends on number of adapters |
| EnumerateModes | 20-100ms | Per display |
| FindDxgiOutputByName | 100-300ms | Full DXGI enumeration |
| GetSystemInfo | < 10ms | Cached after first call |
| GetVersion | < 1ms | Static data |

---

## Error Handling

Query operations are generally safe and do not modify system state:

- Empty results (e.g., no adapters) return empty vectors, not errors
- Invalid indices return `Status::InvalidArg`
- DXGI failures return `Status::DriverError` with detailed messages

---

## Usage Examples

### Example 1: List all displays

```cpp
std::vector<AdapterInfo> adapters;
EnumerateAdapters(adapters);

for (const auto& adapter : adapters) {
    printf("%s: %s (Virtual: %d, Active: %d)\n",
           adapter.deviceInstanceId.c_str(),
           adapter.name.c_str(),
           adapter.isVirtual,
           adapter.isActive);
}
```

### Example 2: Query supported modes

```cpp
std::vector<DisplayMode> modes;
EnumerateModes(0, modes);

for (const auto& mode : modes) {
    printf("%dx%d @ %dHz\n", mode.width, mode.height, 
           mode.refreshNumerator / mode.refreshDenominator);
}
```

### Example 3: Target DXGI output for fullscreen

```cpp
IDXGIOutput* output = nullptr;
if (FindDxgiOutputByName("VDD XR", (void**)&output) == Status::Ok) {
    // Create DXGI swapchain targeting this output
    DXGI_SWAP_CHAIN_DESC desc = {};
    desc.BufferDesc.Width = 1920;
    desc.BufferDesc.Height = 1080;
    desc.BufferDesc.RefreshRate = { 90, 1 };
    desc.Windowed = FALSE;  // Fullscreen
    
    IDXGISwapChain* swapChain;
    factory->CreateSwapChain(device, &desc, &swapChain);
    swapChain->SetFullscreenState(TRUE, output);
}
```

### Example 4: Check system compatibility

```cpp
SystemInfo info;
GetSystemInfo(info);

if (info.osVersion.major < 10 || 
    (info.osVersion.major == 10 && info.osVersion.build < 19041)) {
    printf("ERROR: Windows 10 2004+ required\n");
    return Status::OsUnsupported;
}

printf("System: Windows %d.%d Build %d (%s)\n",
       info.osVersion.major, info.osVersion.minor, 
       info.osVersion.build, info.architecture == "x64" ? "64-bit" : "32-bit");
```

---

## Dependencies

- Windows 10 2004+ for full DisplayConfig support
- DXGI 1.0+ for output enumeration
- User32.dll (EnumDisplayDevices, EnumDisplaySettings)
- ntdll.dll (RtlGetVersion)

---

## Security Considerations

- All query operations are read-only
- No administrator privileges required
- DXGI output pointers must be released by caller (reference counted)

---

## Change History

- **v1.0.0** (2025-11-06): Initial specification based on design document
  - EnumerateAdapters with virtual/physical distinction
  - EnumerateModes for supported resolutions
  - FindDxgiOutputByName for fullscreen targeting
  - GetSystemInfo for compatibility checking
  - GetVersion for SDK version query

