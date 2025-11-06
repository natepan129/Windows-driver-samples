# VDD Query Capability

## Purpose

This delta fixes broken query functions that ignore parameters or return stale data.

---

## MODIFIED Requirements

### Requirement: Display Mode Enumeration

The system SHALL provide a function to list all supported display modes for a given output.

#### Scenario: Enumerate modes for specific output

- **WHEN** `EnumerateModes()` is called with outputIndex parameter
- **THEN** the function maps outputIndex to the correct GDI device name
- **AND** calls EnumDisplaySettingsW with that device name (not nullptr)
- **AND** returns modes supported by the target display, not primary display
- **AND** the modes list is specific to the requested outputIndex

**Changes from v1.0**:
- Fixed: Now respects outputIndex parameter (was always querying primary)
- Fixed: Uses device-specific EnumDisplaySettingsW call
- Result: `vddctl list` now shows correct modes per display

---

## MODIFIED Requirements

### Requirement: State Query Consistency

The system SHALL provide consistent state information across all query functions.

#### Scenario: GetMode queries real-time state

- **WHEN** `GetMode()` is called
- **THEN** the function queries system via EnumDisplaySettingsW(device, ENUM_CURRENT_SETTINGS)
- **AND** does NOT return cached m_activeDisplays data
- **AND** returns the actual current display mode
- **AND** works without requiring prior Activate() call

**Changes from v1.0**:
- Fixed: Now queries live system state instead of stale cache
- Fixed: Works in fresh vddctl process (cache was always empty)
- Result: GetMode returns accurate data

#### Scenario: GetLocation queries real-time state

- **WHEN** `GetLocation()` is called
- **THEN** the function queries system via QueryDisplayConfig or MONITORINFOEX
- **AND** does NOT return cached m_activeDisplays data
- **AND** returns the actual current display position
- **AND** works without requiring prior Activate() call

**Changes from v1.0**:
- Fixed: Now queries live system state instead of stale cache
- Fixed: Consistent with IsActive() (both query real state now)
- Result: State queries are consistent

---

## MODIFIED Requirements

### Requirement: Display Adapter Enumeration

The system SHALL provide a function to list all display adapters (physical and virtual) in the system.

#### Scenario: Identify virtual adapters by Hardware ID

- **WHEN** `EnumerateAdapters()` is called
- **THEN** virtual adapters are identified by Hardware ID match (ROOT\IddSampleDriver)
- **AND** NOT by DeviceString containing "IddSampleDriver"
- **AND** works on all Windows languages
- **AND** isVirtual flag is set correctly

**Changes from v1.0**:
- Fixed: Uses HWID instead of localized DeviceString
- Result: Works on Chinese, Japanese, German Windows

