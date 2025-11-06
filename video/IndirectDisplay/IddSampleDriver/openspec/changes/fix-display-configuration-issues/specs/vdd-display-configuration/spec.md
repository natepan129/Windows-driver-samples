# VDD Display Configuration

## Purpose

This delta fixes critical bugs in display mode, location, and primary designation that cause non-functional APIs and black screens.

---

## MODIFIED Requirements

### Requirement: Primary Display Designation

The system SHALL provide a function to designate a virtual display as the primary display.

#### Scenario: Set virtual display as primary using correct API

- **WHEN** `SetPrimary()` is called on a virtual display
- **THEN** the function uses ChangeDisplaySettingsExW with CDS_SET_PRIMARY flag
- **AND** the virtual display becomes the Windows primary display
- **AND** the taskbar and desktop icons move to the virtual display
- **AND** new windows open on the virtual display by default

**Changes from v1.0**:
- Fixed: Now uses CDS_SET_PRIMARY flag instead of only setting position to (0,0)
- Added: Verification that primary flag was actually set
- Result: SetPrimary actually works now (was completely non-functional in v1.0)

---

## MODIFIED Requirements

### Requirement: Display Mode Configuration

The system SHALL provide a function to change the resolution and refresh rate of a virtual display.

#### Scenario: Set mode preserving full DEVMODE structure

- **WHEN** `SetMode()` is called with a resolution and refresh rate
- **THEN** the function first queries ENUM_CURRENT_SETTINGS
- **AND** preserves all existing DEVMODE fields (orientation, bpp, flags)
- **AND** only modifies dmPelsWidth, dmPelsHeight, dmDisplayFrequency
- **AND** the mode change is accepted by Windows

**Changes from v1.0**:
- Fixed: Now queries current settings first and preserves structure
- Fixed: Handles refresh rate correctly (59.94Hz vs 60Hz)
- Added: Mode validation against supported modes list

#### Scenario: SetDisplayConfig path validation

- **WHEN** `SetMode()` uses SetDisplayConfig method
- **THEN** the function validates modeInfoIdx points to SOURCE mode
- **AND** checks infoType == DISPLAYCONFIG_MODE_INFO_TYPE_SOURCE
- **AND** only modifies source mode fields, not target mode
- **AND** returns error if mode index is invalid or out of range

**Changes from v1.0**:
- Fixed: Added type checking for modeInfoIdx before modification
- Fixed: Distinguishes between SOURCE and TARGET modes
- Added: Error handling for invalid mode indices

---

## MODIFIED Requirements

### Requirement: Display Location Configuration

The system SHALL provide a function to position a virtual display in desktop coordinate space.

#### Scenario: Set location preserving path consistency

- **WHEN** `SetLocation()` is called with new coordinates
- **THEN** the function preserves DISPLAYCONFIG_PATH_ACTIVE flag
- **AND** keeps source-target pairing intact (adapterId, id unchanged)
- **AND** only modifies sourceMode.position (x, y)
- **AND** validates configuration with SDC_VALIDATE before applying

**Changes from v1.0**:
- Fixed: Now preserves path.flags (especially DISPLAYCONFIG_PATH_ACTIVE)
- Fixed: Doesn't modify source/target pairing
- Added: Pre-validation with SDC_VALIDATE
- Added: SDC_ALLOW_CHANGES for flexibility

