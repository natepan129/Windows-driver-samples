# VDD Runtime Control

## Purpose

This capability manages the dynamic activation and deactivation of virtual display outputs without requiring system reboots or driver reinstallation.

---

## Requirements

### Requirement: Virtual Display Activation

The system SHALL provide a function to dynamically create and enable one or more virtual display outputs.

#### Scenario: Activate single virtual display
- **WHEN** `Activate()` is called with a valid display descriptor and count=1
- **THEN** a virtual display device is enabled via DICS_ENABLE
- **AND** the device appears in Windows display topology within 500ms
- **AND** the display has the specified name, resolution, and refresh rate
- **AND** the function returns `Status::Ok`

#### Scenario: Activate multiple virtual displays
- **WHEN** `Activate()` is called with count > 1
- **THEN** the specified number of virtual display outputs are created
- **AND** each display is enumerable via Windows Display Settings
- **AND** each display has a unique connector index

#### Scenario: Activation preserves primary display
- **WHEN** `Activate()` is called on a system with an existing primary display
- **THEN** the current display topology is queried and saved
- **AND** the primary display is verified to remain active after activation
- **AND** if primary display cannot be verified, activation aborts with `Status::DriverError`
- **AND** the existing display topology is reapplied to prevent black screens

#### Scenario: Activation with custom EDID parameters
- **WHEN** `Activate()` is called with HDR10 or stereoscopic flags enabled
- **THEN** the virtual display EDID advertises the specified capabilities
- **AND** Windows reports the display as HDR-capable or 3D-capable accordingly

#### Scenario: Activation when already active
- **WHEN** `Activate()` is called but virtual displays are already active
- **THEN** the function detects the existing state
- **AND** either reuses the existing displays (if parameters match)
- **OR** returns `Status::Busy` if parameters conflict
- **AND** no duplicate displays are created

#### Scenario: Activation without driver installed
- **WHEN** `Activate()` is called but the driver is not installed
- **THEN** the function returns `Status::NotInstalled` immediately
- **AND** provides guidance via `GetLastError()`

---

### Requirement: Virtual Display Deactivation

The system SHALL provide a function to disable all active virtual display outputs.

#### Scenario: Deactivate active displays
- **WHEN** `Deactivate()` is called with active virtual displays
- **THEN** all virtual display devices are disabled via DICS_DISABLE
- **AND** the displays are removed from Windows display topology
- **AND** the desktop is resized to fit remaining physical displays
- **AND** the function returns `Status::Ok`

#### Scenario: Deactivation when not active
- **WHEN** `Deactivate()` is called but no virtual displays are active
- **THEN** the function returns `Status::NotActive`
- **AND** the operation is idempotent (no error state)

#### Scenario: Graceful window migration on deactivation
- **WHEN** `Deactivate()` is called while windows are open on virtual displays
- **THEN** Windows automatically migrates windows to remaining displays
- **AND** the primary display remains functional throughout the process

---

### Requirement: Activation State Query

The system SHALL provide functions to query the current activation state of virtual displays.

#### Scenario: Check if displays are active
- **WHEN** `IsActive()` is called
- **THEN** the function returns `true` if any virtual displays are enabled
- **AND** returns `false` if all virtual displays are disabled or removed

#### Scenario: Query active display count
- **WHEN** `GetActiveDisplayCount()` is called
- **THEN** the function returns the number of currently active virtual displays
- **AND** the count matches the number enumerable via `EnumerateAdapters()`

---

## Implementation Notes

### Primary Display Protection

To prevent black screens on physical machines, `Activate()` implements three safety layers:

1. **Layer 1**: Query and save current display configuration before any changes
2. **Layer 2**: Verify primary display is still active (check for `DISPLAY1` or `DISPLAY2`)
3. **Layer 3**: Reapply configuration with `SDC_ALLOW_CHANGES` flag and error checking

If primary display verification fails, activation is aborted to protect user's display.

### Display Topology Preservation

After enabling the virtual display device:

```cpp
GetDisplayConfigBufferSizes(QDC_ONLY_ACTIVE_PATHS, &numPaths, &numModes);
QueryDisplayConfig(QDC_ONLY_ACTIVE_PATHS, &numPaths, paths, &numModes, modes, nullptr);
SetDisplayConfig(numPaths, paths, numModes, modes, 
                 SDC_APPLY | SDC_USE_SUPPLIED_DISPLAY_CONFIG | 
                 SDC_SAVE_TO_DATABASE | SDC_ALLOW_CHANGES);
```

This forces Windows to re-evaluate topology and keeps all displays enabled.

### Device State Management

Virtual display state is managed through Windows PnP:

- **Enable**: `SetupDiCallClassInstaller(DIF_PROPERTYCHANGE)` with `DICS_ENABLE`
- **Disable**: `SetupDiCallClassInstaller(DIF_PROPERTYCHANGE)` with `DICS_DISABLE`
- **Query**: `CM_Get_DevNode_Status()` to check `DN_STARTED` flag

### Thread Safety

- Internal mutex protects activation/deactivation operations
- Queries (`IsActive`, `GetActiveDisplayCount`) use read-only locks
- State changes are atomic from caller's perspective

---

## Error Recovery

If `Activate()` fails after enabling the device:

1. Windows will attempt to roll back the device to disabled state
2. Primary display should remain functional
3. Call `Deactivate()` to clean up any partial state
4. Check `GetLastError()` for detailed failure reason

Emergency recovery options:

- Press `Win+P` and select "PC screen only" or "Duplicate"
- Run `emergency_recovery.bat` to reset display topology
- Use `DisplaySwitch.exe /internal` to force primary-only mode

---

## Performance Characteristics

| Operation | Typical Duration | Notes |
|-----------|------------------|-------|
| Activate | 500-1000ms | Includes device enable + topology configuration |
| Deactivate | 200-500ms | Device disable only |
| IsActive | < 10ms | Device state query |
| GetActiveDisplayCount | < 10ms | Cached internal state |

---

## Dependencies

- Driver must be installed (`IsDriverInstalled()` returns true)
- SetupAPI for device enable/disable
- DisplayConfig API for topology management
- cfgmgr32.dll for device node status queries

---

## Security Considerations

- Activation/deactivation do not require administrator privileges
- Operations are limited to the installed VDD driver (no arbitrary device control)
- Primary display protection prevents accidental system lock-out

---

## Usage Scenarios

### Scenario 1: VDD as Display Proxy
```cpp
Activate({ .name = "VDD Proxy", .preferredMode = {1920,1080,90,1} });
SetLocation(0, { .x = 1920, .y = 0, .width = 1920, .height = 1080 });
// App renders to VDD; service mirrors to physical screen
```

### Scenario 2: VDD as Secondary Display
```cpp
Activate({ .name = "VDD Remote", .preferredMode = {2560,1440,90,1} });
SetLocation(0, { .x = 3840, .y = 0, .width = 2560, .height = 1440 });
// Remote desktop app captures VDD screen
```

### Scenario 3: VDD as Primary Display
```cpp
Activate({ .name = "VDD Primary", .preferredMode = {1920,1080,60,1} });
SetPrimary(0);
// Apps that only render on primary will use VDD
```

---

## Change History

- **v1.0.0** (2025-11-06): Initial specification based on design document
  - Activate with primary display protection
  - Deactivate with graceful cleanup
  - State query functions (IsActive, GetActiveDisplayCount)
  - Safety mechanisms for physical machine deployment

