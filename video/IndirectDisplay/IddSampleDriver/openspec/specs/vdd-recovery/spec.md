# VDD Recovery Capability

## Purpose

This capability provides automatic recovery mechanisms to clean up orphaned virtual displays and restore system state after application crashes or unexpected failures.

---

## Implementation Status

**Current Version**: v1.0.0 (Planned for v1.1)  
**Status**: Specification complete, implementation pending

The functions defined in this specification (`RecoverOrphanedState()`, `EnsureDriverRunning()`) are currently stubs that return `Status::NotImplemented`. They are planned for implementation in v1.1 release.

**Workaround**: Applications can manually call `Deactivate()` to clean up virtual displays, and use Windows Device Manager or `emergency_recovery.bat` for recovery scenarios.

---

## Requirements

### Requirement: Orphaned State Recovery

The system SHALL provide a function to detect and clean up orphaned virtual displays left behind by crashed applications.

#### Scenario: Recover after application crash
- **WHEN** `RecoverOrphanedState()` is called after an application crash
- **THEN** the function detects virtual displays without active lease owners
- **AND** deactivates all orphaned virtual displays
- **AND** restores the original desktop topology (primary display, positions)
- **AND** returns `Status::Ok` with a summary of recovered items

#### Scenario: Recover when no orphaned state exists
- **WHEN** `RecoverOrphanedState()` is called but all displays have valid leases
- **THEN** the function performs a quick check
- **AND** returns `Status::Ok` without making changes
- **AND** the operation is idempotent and safe to call repeatedly

#### Scenario: Topology restoration
- **WHEN** `RecoverOrphanedState()` detects orphaned VDD was set as primary
- **THEN** the function restores the original physical primary display
- **AND** repositions remaining displays to their original coordinates
- **AND** the desktop size is recalculated to fit remaining displays

#### Scenario: Partial recovery
- **WHEN** `RecoverOrphanedState()` can clean up some but not all orphaned displays
- **THEN** the function proceeds with available cleanup
- **AND** logs which items could not be recovered
- **AND** returns `Status::Ok` with detailed error information

---

### Requirement: Driver Health Check

The system SHALL provide a function to verify the VDD driver stack is running correctly.

#### Scenario: Verify driver running
- **WHEN** `EnsureDriverRunning()` is called and the driver is healthy
- **THEN** the function checks WUDFHost.exe process is running
- **AND** verifies IddSampleDriver device is in DN_STARTED state
- **AND** returns `Status::Ok` immediately

#### Scenario: Restart hung driver
- **WHEN** `EnsureDriverRunning()` detects the driver is hung or crashed
- **THEN** the function attempts to restart the UMDF device via pnputil or devcon
- **AND** waits up to 5 seconds for device to reach DN_STARTED state
- **AND** returns `Status::Ok` if restart succeeds, `Status::DriverError` otherwise

#### Scenario: Driver not installed
- **WHEN** `EnsureDriverRunning()` is called but the driver is not installed
- **THEN** the function returns `Status::NotInstalled`
- **AND** provides guidance to call `InstallDriver()` first

#### Scenario: Verify UMDF host process
- **WHEN** `EnsureDriverRunning()` checks WUDFHost.exe
- **THEN** the function enumerates processes for "WUDFHost.exe"
- **AND** verifies command line contains "IddSampleDriver"
- **AND** checks process is not in zombie state (has > 0 threads)

---

### Requirement: Display Topology Backup

The system SHALL automatically capture display topology before risky operations to enable restoration.

#### Scenario: Backup before activation
- **WHEN** `Activate()` or `ActivateLeased()` is called
- **THEN** the system captures current display configuration:
  - Primary display index
  - Each display's desktop rectangle (x, y, width, height)
  - Each display's mode (resolution, refresh rate)
  - Display path topology (extended vs clone)
- **AND** stores the backup in memory (linked to lease) or broker registry

#### Scenario: Restore on crash
- **WHEN** a leased session times out due to missed heartbeats
- **THEN** the broker automatically restores the backed-up topology
- **AND** deactivates virtual displays
- **AND** reapplies the saved configuration via `SetDisplayConfig`

#### Scenario: Restore on explicit recovery
- **WHEN** `RecoverOrphanedState()` is called
- **THEN** the function attempts to load saved topology from broker registry
- **AND** applies the restoration if available
- **AND** falls back to "safe mode" (primary only) if no backup exists

---

### Requirement: Safe Mode Fallback

The system SHALL provide a guaranteed safe fallback to prevent permanent black screens.

#### Scenario: Emergency safe mode
- **WHEN** `RecoverOrphanedState()` cannot restore original topology
- **THEN** the function forces display configuration to "PC screen only" mode
- **AND** disables all virtual displays
- **AND** ensures at least one physical display is active and set as primary

#### Scenario: Safe mode via Win+P
- **WHEN** recovery functions are unavailable (e.g., SDK not accessible)
- **THEN** users can press Win+P and select "PC screen only"
- **AND** Windows automatically disables extension and virtual displays

#### Scenario: Safe mode via DisplaySwitch.exe
- **WHEN** recovery is needed via script or remote access
- **THEN** execute `DisplaySwitch.exe /internal` to force internal display only
- **AND** this works even if SDK is unresponsive

---

## Implementation Notes

### Orphaned Display Detection

`RecoverOrphanedState()` uses the following detection algorithm:

```cpp
1. Query broker for all active leases
2. For each active VDD device:
   a. Check if owned by a valid lease
   b. If lease exists, check if process is still running
   c. If process is dead or lease timed out, mark as orphaned
3. For orphaned displays:
   a. Deactivate via SetupDiCallClassInstaller(DIF_PROPERTYCHANGE, DICS_DISABLE)
   b. Load backed-up topology from registry
   c. Restore via SetDisplayConfig
   d. If restore fails, fall back to DisplaySwitch /internal
```

### Driver Health Monitoring

`EnsureDriverRunning()` performs these checks:

```cpp
1. Check device status:
   ULONG status, problemNumber;
   CM_Get_DevNode_Status(&status, &problemNumber, deviceInst, 0);
   if (!(status & DN_STARTED) || (status & DN_HAS_PROBLEM)) {
       // Device not running
   }

2. Check UMDF host process:
   - Find WUDFHost.exe process(es)
   - Verify command line contains "IddSampleDriver.dll"
   - Check process thread count > 0

3. Attempt restart if needed:
   - pnputil /restart-device <instance-id>
   - Wait for DN_STARTED (poll every 100ms, timeout 5s)
```

### Topology Backup Storage

**In-Memory (SDK)**
- Stored in `VddSdkImpl::m_topologyBackup` structure
- Lost if SDK process crashes (hence need broker persistence)

**Broker Registry (persistent)**
- Stored in `HKLM\SOFTWARE\VDD\TopologyBackup\{LeaseId}`
- JSON format:
  ```json
  {
    "timestamp": "2025-11-06T12:00:00Z",
    "primaryIndex": 0,
    "displays": [
      { "deviceName": "\\\\.\\DISPLAY1", "x": 0, "y": 0, "width": 1920, "height": 1080, "refreshRate": 60 },
      ...
    ]
  }
  ```

### Safe Mode Implementation

**Method 1: DisplaySwitch.exe** (most reliable)
```cpp
system("DisplaySwitch.exe /internal");  // Force internal display only
```

**Method 2: SetDisplayConfig** (programmatic)
```cpp
// Query physical displays only
QueryDisplayConfig(QDC_ONLY_ACTIVE_PATHS, ...);
// Filter out virtual displays
// Apply configuration with SDC_TOPOLOGY_INTERNAL
SetDisplayConfig(numPaths, filteredPaths, numModes, modes, 
                 SDC_TOPOLOGY_INTERNAL | SDC_APPLY);
```

**Method 3: Win+P keyboard shortcut** (manual)
- User presses Win+P
- Selects "PC screen only"
- Windows handles everything automatically

---

## Error Handling

Recovery operations are designed to be fault-tolerant:

| Condition | Behavior |
|-----------|----------|
| No orphaned displays | Quick check, returns `Status::Ok` |
| Partial cleanup success | Continues, reports what was cleaned |
| Topology restore fails | Falls back to safe mode |
| Driver restart fails | Reports error but doesn't fail catastrophically |
| Broker unavailable | Uses local detection (process enumeration) |

---

## Performance Characteristics

| Operation | Typical Duration | Notes |
|-----------|------------------|-------|
| RecoverOrphanedState | 500-2000ms | Depends on number of orphaned displays |
| EnsureDriverRunning | 50-5000ms | Fast if healthy; slow if restart needed |
| TopologyBackup (capture) | 100-300ms | DisplayConfig query |
| TopologyRestore | 300-1000ms | SetDisplayConfig application |
| SafeMode fallback | 500-1500ms | DisplaySwitch.exe execution |

---

## Usage Examples

### Example 1: Startup recovery

```cpp
// On application startup, clean up any leftovers
Status result = RecoverOrphanedState();
if (result == Status::Ok) {
    printf("Recovery check passed or cleanup successful\n");
} else {
    printf("Recovery encountered issues: %s\n", GetLastError().c_str());
}

// Verify driver is healthy before proceeding
if (EnsureDriverRunning() != Status::Ok) {
    printf("Driver not running, please reinstall\n");
    return -1;
}

// Now safe to activate
BeginSession(lease);
ActivateLeased({ ... }, lease);
```

### Example 2: Periodic health monitoring

```cpp
// Background thread checks driver health every 30 seconds
while (appRunning) {
    std::this_thread::sleep_for(std::chrono::seconds(30));
    
    if (EnsureDriverRunning() != Status::Ok) {
        printf("WARNING: Driver became unhealthy, attempting recovery...\n");
        RecoverOrphanedState();
        
        // Try to reactivate
        if (ActivateLeased({ ... }, lease) != Status::Ok) {
            printf("ERROR: Could not recover display\n");
            // Notify user, gracefully degrade, etc.
        }
    }
}
```

### Example 3: Emergency recovery script

```batch
@echo off
REM emergency_recovery.bat

echo [1] Deactivating virtual displays...
vddctl.exe deactivate

echo [2] Running orphaned state recovery...
vddctl.exe recover-orphaned

echo [3] Forcing safe mode (internal display only)...
DisplaySwitch.exe /internal

timeout /t 2

echo [4] Verifying driver status...
vddctl.exe ensure-driver-running

echo Recovery complete. Check display settings.
pause
```

---

## Dependencies

- Broker service (for persistent topology backups)
- SetupAPI/cfgmgr32 for device status queries
- DisplayConfig API for topology management
- Process enumeration APIs (CreateToolhelp32Snapshot, EnumProcesses)

---

## Security Considerations

- `RecoverOrphanedState()` does not require admin privileges (only deactivates, no install/uninstall)
- `EnsureDriverRunning()` restart functionality requires admin for device manipulation
- Emergency recovery scripts should be run as administrator for full access

---

## Monitoring and Diagnostics

### Recommended Logging

Recovery operations should log:

- Timestamp of recovery attempt
- List of orphaned displays detected
- Backup topology loaded (if any)
- Restoration success/failure per display
- Driver restart attempts and outcomes

### Metrics to Track

- Number of recovery calls per application session
- Success rate of topology restoration
- Driver restart frequency
- Time to recovery completion

---

## Change History

- **v1.0.1** (2025-11-06): Added implementation status note
  - Marked functions as "Planned for v1.1"
  - Added workaround information for current version
- **v1.0.0** (2025-11-06): Initial specification based on design document
  - RecoverOrphanedState for crash cleanup
  - EnsureDriverRunning for driver health verification
  - Topology backup and restoration mechanisms
  - Safe mode fallback (DisplaySwitch.exe, Win+P)
  - Emergency recovery scripts

