# VDD Display Configuration

## Purpose

This capability provides functions to configure virtual display parameters including resolution, refresh rate, position in desktop space, and primary display designation.

---

## Requirements

### Requirement: Display Mode Configuration

The system SHALL provide a function to change the resolution and refresh rate of a virtual display.

#### Scenario: Set standard resolution
- **WHEN** `SetMode()` is called with a standard resolution (e.g., 1920x1080@60Hz)
- **THEN** the system attempts mode change via `SetDisplayConfig` API first
- **AND** if that fails, falls back to `ChangeDisplaySettingsExW`
- **AND** the virtual display adopts the new mode within 1 second
- **AND** the function returns `Status::Ok`

#### Scenario: Set custom resolution
- **WHEN** `SetMode()` is called with a custom resolution (e.g., 2560x1440@90Hz)
- **THEN** the system validates the mode against driver-advertised modes
- **AND** applies the mode if supported by EDID
- **AND** returns `Status::InvalidArg` if mode is not supported

#### Scenario: Set mode on inactive display
- **WHEN** `SetMode()` is called on a virtual display that is not in active topology
- **THEN** the system attempts to activate the display path first
- **AND** if activation fails, returns detailed error via `GetLastError()`
- **AND** suggests using Windows Display Settings to detect/extend displays

#### Scenario: Mode change with dual strategy
- **WHEN** `SetMode()` is called and modern SetDisplayConfig fails
- **THEN** the system automatically falls back to legacy ChangeDisplaySettingsExW
- **AND** applies changes with CDS_UPDATEREGISTRY | CDS_NORESET
- **AND** commits changes with a final ChangeDisplaySettingsExW(nullptr) call

---

### Requirement: Display Location Configuration

The system SHALL provide a function to position a virtual display in desktop coordinate space.

#### Scenario: Position display to the right of primary
- **WHEN** `SetLocation()` is called with x=3840, y=0 (assuming primary is 3840x2160)
- **THEN** the virtual display is positioned as an extended desktop to the right
- **AND** the mouse can move seamlessly from primary to virtual display
- **AND** the change persists across reboots (saved to display database)

#### Scenario: Position display above primary
- **WHEN** `SetLocation()` is called with x=0, y=-1080 (negative Y)
- **THEN** the virtual display is positioned above the primary
- **AND** windows can be dragged to the virtual display
- **AND** taskbar remains on primary display by default

#### Scenario: Reposition overlapping displays
- **WHEN** `SetLocation()` is called and the new position overlaps another display
- **THEN** Windows automatically adjusts positions to avoid overlap (if possible)
- **OR** allows overlap if explicitly configured via topology flags

---

### Requirement: Primary Display Designation

The system SHALL provide a function to designate a virtual display as the primary display.

#### Scenario: Set virtual display as primary
- **WHEN** `SetPrimary()` is called on a virtual display
- **THEN** the virtual display becomes the primary in Windows display topology
- **AND** the taskbar and desktop icons move to the virtual display
- **AND** new windows open on the virtual display by default
- **AND** the previous primary becomes a secondary display

#### Scenario: Set primary on a physical display after using VDD
- **WHEN** `SetPrimary()` is called on a physical display (index != VDD)
- **THEN** the SDK correctly identifies the physical display device name
- **AND** updates the display path priority to make it primary
- **AND** the virtual display becomes secondary

#### Scenario: Restore original primary after deactivation
- **WHEN** `Deactivate()` is called after VDD was set as primary
- **THEN** Windows automatically restores the original primary display
- **AND** taskbar and icons return to the physical display

---

### Requirement: Display Mode Query

The system SHALL provide a function to retrieve the current display mode of a virtual display.

#### Scenario: Query current mode
- **WHEN** `GetMode()` is called on an active virtual display
- **THEN** the function returns the current resolution and refresh rate
- **AND** the values match what Windows Display Settings shows

#### Scenario: Query mode on inactive display
- **WHEN** `GetMode()` is called on an inactive virtual display
- **THEN** the function returns `Status::NotActive`
- **AND** provides guidance via `GetLastError()`

---

### Requirement: Display Location Query

The system SHALL provide a function to retrieve the current desktop position of a virtual display.

#### Scenario: Query current location
- **WHEN** `GetLocation()` is called on an active virtual display
- **THEN** the function returns the current desktop coordinates (x, y, width, height)
- **AND** the coordinates are in virtual desktop space (pixels)

---

## Implementation Notes

### Virtual Display Identification

To correctly target virtual displays, the SDK implements `GetVirtualDisplayDeviceNames()` helper:

```cpp
std::vector<std::wstring> GetVirtualDisplayDeviceNames() {
    // Enumerate all displays
    // Check DeviceString for "IddSampleDriver" or "Generic PnP Monitor"
    // Check Monitor DeviceID for EDID patterns (DELD0E6, LEN65BF, HWP2676)
    // Return matching device names (e.g., \\.\DISPLAY38)
}
```

This ensures operations like `SetMode(0, ...)` correctly target the first VDD, not a physical display.

### Dual-Strategy Mode Switching

`SetMode` implements a two-phase approach for maximum compatibility:

**Phase 1: Modern API (SetDisplayConfig)**
- Query current paths via `QueryDisplayConfig(QDC_ONLY_ACTIVE_PATHS)`
- Find target display by matching device name
- Modify source mode width/height in `modeInfoArray`
- Apply via `SetDisplayConfig(SDC_APPLY | SDC_USE_SUPPLIED_DISPLAY_CONFIG | SDC_SAVE_TO_DATABASE)`

**Phase 2: Legacy API (ChangeDisplaySettingsExW)** (fallback)
- Query current settings via `EnumDisplaySettingsW(ENUM_CURRENT_SETTINGS)`
- Modify `DEVMODE` structure (dmPelsWidth, dmPelsHeight, dmDisplayFrequency)
- Apply via `ChangeDisplaySettingsExW(deviceName, &devMode, nullptr, CDS_UPDATEREGISTRY | CDS_NORESET)`
- Commit via `ChangeDisplaySettingsExW(nullptr, nullptr, nullptr, 0)`

### Topology Management

All configuration functions use `SetDisplayConfig` with these flags:

- `SDC_APPLY`: Apply changes immediately
- `SDC_USE_SUPPLIED_DISPLAY_CONFIG`: Use provided paths/modes
- `SDC_SAVE_TO_DATABASE`: Persist changes across reboots
- `SDC_ALLOW_CHANGES`: Allow Windows to adjust topology if needed
- `SDC_TOPOLOGY_SUPPLIED`: Explicitly provide topology (for primary changes)

### Auto-Initialization

Configuration functions (`SetMode`, `SetLocation`, `SetPrimary`) automatically create the SDK instance if not initialized:

```cpp
Status SetMode(uint32_t outputIndex, const DisplayMode& mode) {
    std::lock_guard<std::mutex> lock(g_instanceMutex);
    VddSdkImpl* impl = GetOrCreateInstance_Locked();  // Auto-create
    return impl->SetMode(outputIndex, mode);
}
```

This allows standalone usage without explicit `Initialize()` calls.

---

## Error Handling

Common error scenarios:

| Error | Cause | Resolution |
|-------|-------|------------|
| `DISP_CHANGE_BADMODE` | Unsupported resolution/refresh rate | Check EDID-advertised modes via `EnumerateModes()` |
| `DISP_CHANGE_FAILED` | Display not in active topology | Open Windows Display Settings > Detect/Extend |
| `modeInfoIdx = -1` | Display path not active | Use `SetDisplayConfig` to activate path first |
| `Status::NotInstalled` | Driver not installed | Call `InstallDriver()` first |

VirtualBox Limitation: Virtual displays may not appear in Windows Display Settings due to VM display management layer, but API calls will succeed.

---

## Performance Characteristics

| Operation | Typical Duration | Notes |
|-----------|------------------|-------|
| SetMode | 500-1500ms | Includes display mode switch + Windows adjustment |
| SetLocation | 200-500ms | Topology reconfiguration |
| SetPrimary | 300-800ms | Primary designation + taskbar migration |
| GetMode | < 10ms | Query only |
| GetLocation | < 10ms | Query only |

---

## Dependencies

- Active virtual displays (call `Activate()` first)
- DisplayConfig API (Windows 10+)
- User32.dll (ChangeDisplaySettingsExW)

---

## Usage Examples

### Example 1: Configure VDD as extended display

```cpp
Activate({ .name = "VDD XR", .preferredMode = {1920,1080,90,1} });
SetLocation(0, { .x = 3840, .y = 0, .width = 1920, .height = 1080 });
SetMode(0, { .width = 2560, .height = 1440, .refreshNumerator = 90, .refreshDenominator = 1 });
```

### Example 2: Make VDD the primary display

```cpp
Activate({ .name = "VDD Primary", .preferredMode = {1920,1080,60,1} });
SetPrimary(0);  // VDD becomes primary
// Launch app → it renders on VDD by default
```

### Example 3: Query and restore configuration

```cpp
// Save original state
DisplayMode originalMode;
DisplayRect originalRect;
GetMode(0, originalMode);
GetLocation(0, originalRect);

// Make changes...
SetMode(0, { .width = 1024, .height = 768, .refreshNumerator = 60, .refreshDenominator = 1 });

// Restore
SetMode(0, originalMode);
SetLocation(0, originalRect);
```

---

## Change History

- **v1.0.0** (2025-11-06): Initial specification based on design document
  - SetMode with dual-strategy (SetDisplayConfig + ChangeDisplaySettingsExW fallback)
  - SetLocation for desktop positioning
  - SetPrimary for primary display designation
  - GetMode and GetLocation query functions
  - Virtual display identification helper (GetVirtualDisplayDeviceNames)
  - Auto-initialization support

