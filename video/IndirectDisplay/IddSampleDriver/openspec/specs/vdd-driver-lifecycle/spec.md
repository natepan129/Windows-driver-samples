# VDD Driver Lifecycle

## Purpose

This capability manages the installation, uninstallation, and versioning of the Virtual Display Driver (VDD) UMDF package on Windows systems.

---

## Requirements

### Requirement: Driver Installation

The system SHALL provide a function to install the UMDF Indirect Display Driver package from an INF file.

#### Scenario: Successful installation with valid INF
- **WHEN** `InstallDriver()` is called with a valid INF path and administrator privileges
- **THEN** the UMDF driver package is staged to the Driver Store
- **AND** the device is registered and activated
- **AND** the function returns `Status::Ok`
- **AND** the driver appears in Device Manager

#### Scenario: Installation without administrator privileges
- **WHEN** `InstallDriver()` is called without administrator privileges
- **THEN** the installation fails immediately
- **AND** the function returns `Status::AdminRequired`
- **AND** no system state is modified

#### Scenario: Installation with invalid INF path
- **WHEN** `InstallDriver()` is called with a non-existent or invalid INF path
- **THEN** the installation fails validation
- **AND** the function returns `Status::InvalidArg`
- **AND** detailed error information is available via `GetLastError()`

#### Scenario: Installation rollback on failure
- **WHEN** `InstallDriver()` registers a device but driver installation fails
- **THEN** the system automatically removes the partially registered device
- **AND** the function returns `Status::DriverError`
- **AND** no orphaned device nodes remain in the system

#### Scenario: Duplicate installation prevention
- **WHEN** `InstallDriver()` is called but the driver is already installed
- **THEN** the function detects the existing installation
- **AND** returns `Status::AlreadyInstalled`
- **AND** the existing installation remains unchanged

---

### Requirement: Driver Uninstallation

The system SHALL provide a function to completely remove the UMDF Indirect Display Driver from the system.

#### Scenario: Complete uninstallation
- **WHEN** `UninstallDriver()` is called with administrator privileges
- **THEN** all virtual display devices are enumerated and removed
- **AND** the driver package is removed from the Driver Store
- **AND** all registry entries are cleaned up
- **AND** the function returns `Status::Ok`

#### Scenario: Uninstallation when driver not installed
- **WHEN** `UninstallDriver()` is called but no driver is installed
- **THEN** the function returns `Status::NotInstalled`
- **AND** no error is raised (idempotent operation)

#### Scenario: Partial uninstallation handling
- **WHEN** `UninstallDriver()` encounters failures removing some devices
- **THEN** the function continues attempting to remove remaining devices
- **AND** returns `Status::Ok` if at least one device was removed
- **AND** provides detailed status via `GetLastError()`

#### Scenario: Safe device removal sequence
- **WHEN** `UninstallDriver()` removes devices
- **THEN** each device is first disabled (DICS_DISABLE)
- **AND** then removed (DIF_REMOVE)
- **AND** the display topology is preserved for remaining displays

---

### Requirement: Driver Detection

The system SHALL provide a function to query whether the VDD driver is currently installed.

#### Scenario: Detect installed driver
- **WHEN** `IsDriverInstalled()` is called and the driver is installed
- **THEN** the function queries multiple sources (Device Manager, Driver Store, Services)
- **AND** returns `true` if any VDD device is found
- **AND** the operation completes quickly (< 100ms)

#### Scenario: Detect no driver
- **WHEN** `IsDriverInstalled()` is called and no driver is installed
- **THEN** the function returns `false`
- **AND** the operation completes quickly (< 100ms)

---

### Requirement: Driver Version Query

The system SHALL provide a function to retrieve the installed driver version.

#### Scenario: Query driver version
- **WHEN** `GetDriverVersion()` is called and the driver is installed
- **THEN** the function returns the version from the driver INF
- **AND** the version follows semantic versioning (major.minor.patch)

#### Scenario: Query version when not installed
- **WHEN** `GetDriverVersion()` is called but no driver is installed
- **THEN** the function returns `Status::NotInstalled`
- **AND** provides appropriate error details

---

## Implementation Notes

### Robust Rollback Mechanism

The `InstallDriver()` function implements a multi-stage rollback mechanism:

1. **Pre-registration phase**: Failures cause immediate return with no side effects
2. **Post-registration phase**: Any failure after device registration triggers automatic cleanup
3. **Cleanup includes**: Remove device node, clear registry entries, release all handles

### Multiple Detection Sources

`IsDriverInstalled()` checks multiple sources for reliability:

- Device Manager enumeration (HWID: `ROOT\IddSampleDriver`)
- Driver Store query (INF package presence)
- Windows Service status (WUDFHost running with VDD)
- Registry keys (`HKLM\SYSTEM\CurrentControlSet\Services`)

### Thread Safety

All driver lifecycle operations are protected by internal mutexes to ensure:

- Concurrent install/uninstall operations are serialized
- Detection queries can run concurrently with each other
- No race conditions between install and status checks

---

## Dependencies

- Windows 10 2004+ / Windows 11 (WDDM 2.x)
- Windows Driver Kit (WDK) for driver compilation
- Administrator privileges for install/uninstall operations
- SetupAPI/newdev.dll for driver management
- cfgmgr32.dll for device node operations

---

## Error Handling

All functions return a `Status` enum and populate a thread-local error string accessible via `GetLastError()`:

```cpp
Status result = InstallDriver(L"driver.inf");
if (result != Status::Ok) {
    std::string details = GetLastError();  // Human-readable error
    // Handle error...
}
```

---

## Security Considerations

- **UAC Elevation**: Install/uninstall require administrator privileges
- **INF Validation**: Path validation prevents directory traversal attacks
- **Atomic Operations**: Rollback mechanism prevents partial installation states
- **Test Signing**: Development mode requires `bcdedit /set testsigning on`
- **Production Signing**: Requires EV code signing certificate + Microsoft attestation

---

## Performance Characteristics

| Operation | Typical Duration | Notes |
|-----------|------------------|-------|
| InstallDriver | 2-5 seconds | Includes Driver Store staging + device registration |
| UninstallDriver | 1-3 seconds | Per device; may be longer with multiple devices |
| IsDriverInstalled | < 100ms | Cached after first query |
| GetDriverVersion | < 50ms | Registry read only |

---

## Change History

- **v1.0.0** (2025-11-06): Initial specification based on design document
  - InstallDriver with automatic rollback
  - UninstallDriver with complete cleanup
  - IsDriverInstalled with multi-source detection
  - GetDriverVersion query support

