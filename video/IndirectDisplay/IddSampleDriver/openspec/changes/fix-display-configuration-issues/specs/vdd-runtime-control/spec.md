# VDD Runtime Control

## Purpose

This delta addresses critical topology corruption issues in virtual display activation.

---

## MODIFIED Requirements

### Requirement: Virtual Display Activation

The system SHALL provide a function to dynamically create and enable one or more virtual display outputs.

#### Scenario: Activation preserves primary display

- **WHEN** `Activate()` is called on a system with an existing primary display
- **THEN** the function uses explicit topology strategy (SDC_TOPOLOGY_EXTEND)
- **AND** does NOT use blind QueryDisplayConfig replay
- **AND** the primary display remains active and functional
- **AND** no black screen occurs during activation

**Changes from v1.0**:
- Fixed: Removed dangerous blind query-and-replay pattern
- Added: Explicit SDC_TOPOLOGY_EXTEND flag
- Added: Validation before applying configuration (SDC_VALIDATE)

#### Scenario: Device stabilization with polling

- **WHEN** `Activate()` enables a virtual display device
- **THEN** the function polls device status until DN_STARTED
- **AND** polls display configuration until source appears
- **AND** waits up to 5 seconds (not fixed 500ms)
- **AND** returns error if device doesn't stabilize in time

**Changes from v1.0**:
- Fixed: Replaced fixed 500ms delay with intelligent polling
- Added: CM_Get_DevNode_Status polling loop
- Added: QueryDisplayConfig source appearance check

---

## MODIFIED Requirements

### Requirement: Device Identification

The system SHALL reliably identify virtual display devices across all Windows configurations.

#### Scenario: Hardware ID-based identification

- **WHEN** the system needs to identify virtual displays
- **THEN** it uses Hardware ID (ROOT\IddSampleDriver) matching via SetupAPI
- **AND** does NOT rely on DeviceString or EDID fragments
- **AND** works on all Windows languages (English, Chinese, Japanese, etc.)
- **AND** works with OEM-customized drivers

**Changes from v1.0**:
- Fixed: Removed string-based device matching (DeviceString, EDID patterns)
- Added: Hardware ID-based enumeration via SetupAPI
- Added: Device instance to GDI name mapping via DisplayConfig

#### Scenario: Multi-string Hardware ID parsing

- **WHEN** querying SPDRP_HARDWAREID for device identification
- **THEN** the function iterates through entire REG_MULTI_SZ
- **AND** checks all compatible IDs, not just the first
- **AND** finds device regardless of HWID order

**Changes from v1.0**:
- Fixed: SPDRP_HARDWAREID now correctly parsed as MULTI_SZ
- Added: Loop through all strings in MULTI_SZ array

