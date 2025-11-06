# VDD SDK OpenSpec Documentation

## Overview

This directory contains the complete OpenSpec-compliant specifications for the Virtual Display Driver (VDD) SDK project, based on the comprehensive design document.

**Status**: ✅ All specifications validated and passing  
**Version**: 1.0.0  
**Last Updated**: 2025-11-06  

---

## Project Structure

```
openspec/
├── AGENTS.md              # AI agent instructions for OpenSpec workflow
├── project.md             # Project conventions and overview
├── README.md              # This file
├── specs/                 # Current specifications (what IS built)
│   ├── vdd-driver-lifecycle/
│   │   └── spec.md        # Driver install/uninstall/detection
│   ├── vdd-runtime-control/
│   │   └── spec.md        # Activate/deactivate virtual displays
│   ├── vdd-display-configuration/
│   │   └── spec.md        # Mode/location/primary configuration
│   ├── vdd-query/
│   │   └── spec.md        # Enumerate adapters/modes/outputs
│   └── vdd-recovery/
│       └── spec.md        # Orphaned state cleanup & health checks
└── changes/
    ├── add-session-management/  # ACTIVE: Broker service proposal
    │   ├── proposal.md
    │   ├── tasks.md
    │   ├── design.md
    │   └── specs/
    │       └── vdd-session-management/
    │           └── spec.md  # ADDED Requirements
    └── archive/           # Completed changes (empty for initial release)
```

---

## Capabilities Summary

### ✅ 1. Driver Lifecycle (`vdd-driver-lifecycle`)

**Purpose**: Manage UMDF driver installation, uninstallation, and versioning

**Key Requirements**:
- Driver Installation with automatic rollback on failure
- Complete driver uninstallation with device cleanup
- Multi-source driver detection (Device Manager, Driver Store, Services)
- Driver version query

**Status**: Fully implemented and documented

**Primary APIs**:
- `InstallDriver(infPath)` - Install with admin elevation
- `UninstallDriver()` - Complete cleanup
- `IsDriverInstalled()` - Detection query
- `GetDriverVersion()` - Version query

---

### ✅ 2. Runtime Control (`vdd-runtime-control`)

**Purpose**: Dynamic activation/deactivation of virtual displays without reboots

**Key Requirements**:
- Virtual display activation with primary display protection
- Graceful deactivation with window migration
- Activation state query
- Safety mechanisms for physical machines

**Status**: Fully implemented with 3-layer safety protection

**Primary APIs**:
- `Activate(desc, count)` - Enable virtual displays
- `Deactivate()` - Disable virtual displays
- `IsActive()` - Check active state
- `GetActiveDisplayCount()` - Count active displays

**Safety Features**:
- Layer 1: Save current configuration before changes
- Layer 2: Verify primary display remains active
- Layer 3: Reapply configuration with error checking

---

### ✅ 3. Display Configuration (`vdd-display-configuration`)

**Purpose**: Configure virtual display parameters (resolution, position, primary)

**Key Requirements**:
- Display mode configuration with dual-strategy approach
- Desktop position/location management
- Primary display designation
- Mode and location query

**Status**: Fully implemented with fallback mechanisms

**Primary APIs**:
- `SetMode(index, mode)` - Change resolution/refresh rate
- `SetLocation(index, rect)` - Position in desktop space
- `SetPrimary(index)` - Designate as primary
- `GetMode(index)` - Query current mode
- `GetLocation(index)` - Query current position

**Dual Strategy**:
1. Modern API: `SetDisplayConfig` (Windows 10+)
2. Fallback: `ChangeDisplaySettingsExW` (compatibility)

---

### ✅ 4. Query (`vdd-query`)

**Purpose**: Enumerate and query display adapters, modes, and DXGI outputs

**Key Requirements**:
- Display adapter enumeration (physical + virtual)
- Supported display mode enumeration
- DXGI output discovery for fullscreen targeting
- System information query

**Status**: Fully implemented

**Primary APIs**:
- `EnumerateAdapters(out)` - List all adapters
- `EnumerateModes(index, out)` - List supported modes
- `FindDxgiOutputByName(name, out)` - Find DXGI output
- `GetSystemInfo(out)` - System capabilities
- `GetVersion()` - SDK version

---

### ⏳ 5. Session Management (`add-session-management` - CHANGE PROPOSAL)

**Purpose**: Fault-tolerant sessions with leasing, heartbeat, and auto-recovery

**Key Requirements**:
- Session creation with lease handles
- Leased activation with auto-restore on crash
- Heartbeat monitoring (1-2s intervals, 5s timeout)
- Graceful session termination
- State query

**Status**: Change proposal created, implementation planned for v1.1  
**Location**: `changes/add-session-management/`

**Primary APIs** (planned):
- `BeginSession(out)` - Create lease
- `ActivateLeased(opts, lease)` - Activate under lease
- `Heartbeat(lease)` - Keep session alive
- `EndSession(lease)` - Release lease
- `GetState(out)` - Query session state

**Dependencies**: VddSvc.exe broker service (planned for v1.1)

---

### ⏳ 6. Recovery (`vdd-recovery`)

**Purpose**: Automatic cleanup and health monitoring

**Key Requirements**:
- Orphaned state detection and cleanup
- Driver health verification and restart
- Display topology backup/restore
- Safe mode fallback mechanisms

**Status**: Partially implemented (core recovery done, broker integration pending)

**Primary APIs**:
- `RecoverOrphanedState()` - Clean up abandoned displays ✅
- `EnsureDriverRunning()` - Verify driver health ✅ (restart needs admin)
- Topology backup - Requires broker service ⏳
- Safe mode fallback - Emergency scripts available ✅

---

## Design Document Mapping

This OpenSpec structure directly implements the design document provided:

| Design Doc Section | OpenSpec Capability | Status |
|-------------------|-------------------|--------|
| **§1 Architecture** | All capabilities | ✅ Implemented |
| **§2 Feature Map** | Driver Lifecycle, Runtime Control, Configuration | ✅ Implemented |
| **§3 API Design** | All 6 capabilities | ✅ API complete |
| **§4 vddctl CLI** | Documented in project.md | ✅ CLI tool complete |
| **Fault-tolerant session** | Session Management | ⏳ Broker pending |
| **Recovery helpers** | Recovery | ✅ Core done, broker pending |

---

## Three Target Scenarios

The specifications support all three design document scenarios:

### Scenario 1: VDD as Display Proxy
**Specs**: Runtime Control + Display Configuration  
**Usage**: Full-screen 3D on physical screen with VDD as intermediary

```cpp
Activate({ .name = "VDD Proxy", .preferredMode = {1920,1080,90,1} });
SetLocation(0, { .x = 1920, .y = 0 });
// App renders to VDD; service mirrors to physical screen
```

### Scenario 2: VDD as Secondary Display
**Specs**: Runtime Control + Display Configuration  
**Usage**: Virtual monitor for remote desktop apps

```cpp
Activate({ .name = "VDD Remote", .preferredMode = {2560,1440,90,1} });
SetLocation(0, { .x = 3840, .y = 0 });
// Remote app captures VDD screen
```

### Scenario 3: VDD as Primary Display
**Specs**: Runtime Control + Display Configuration  
**Usage**: Apps that only render on primary

```cpp
Activate({ .name = "VDD Primary", .preferredMode = {1920,1080,60,1} });
SetPrimary(0);
// App fullscreen to VDD by default
```

---

## Implementation Status

### Completed (v1.0.0)

- ✅ **Driver Lifecycle**: Full implementation with rollback
- ✅ **Runtime Control**: Activate/deactivate with safety
- ✅ **Display Configuration**: Mode/location/primary with dual strategy
- ✅ **Query**: Enumerate adapters, modes, DXGI outputs
- ✅ **Recovery (Core)**: OrphanedState cleanup, driver health check
- ✅ **CLI Tool**: vddctl.exe with JSON output
- ✅ **Safety Mechanisms**: Primary protection, emergency recovery

### Planned (v1.1.0)

- ⏳ **Broker Service**: VddSvc.exe for session management
- ⏳ **Session Management**: Full implementation with leasing
- ⏳ **Topology Backup**: Persistent state across broker restarts
- ⏳ **Enhanced Recovery**: Automatic restore after crash

---

## Validation

All specifications have been validated with OpenSpec:

```bash
$ openspec validate --specs --strict
✓ spec/vdd-display-configuration
✓ spec/vdd-driver-lifecycle
✓ spec/vdd-query
✓ spec/vdd-recovery
✓ spec/vdd-runtime-control
Totals: 5 passed, 0 failed (5 items)

$ openspec validate add-session-management --strict
✓ Change 'add-session-management' is valid
```

---

## Quick Start

### View All Specifications

```bash
openspec list --specs
```

### View Specific Capability

```bash
openspec show vdd-driver-lifecycle --type spec
openspec show vdd-runtime-control --type spec
```

### Search Requirements

```bash
# Find all scenarios related to "primary display"
rg -n "Scenario:.*primary" openspec/specs

# Find all "SHALL" requirements
rg -n "SHALL" openspec/specs
```

---

## Safety & Security

### Safety Mechanisms Documented

- **Primary Display Protection**: Verified in `vdd-runtime-control/spec.md` § "Primary Display Protection"
- **Automatic Rollback**: Specified in `vdd-driver-lifecycle/spec.md` § "Robust Rollback Mechanism"
- **Emergency Recovery**: Detailed in `vdd-recovery/spec.md` § "Safe Mode Fallback"

### Security Model

- **Admin Requirements**: Clearly marked per operation in `project.md` § "Security Model"
- **Driver Signing**: Development (test signing) and production (EV cert) documented
- **Thread Safety**: All APIs thread-safe via internal mutexes
- **Memory Safety**: Clear ownership model documented

---

## Testing Coverage

Each specification includes test scenarios:

- **Positive scenarios**: Expected success paths
- **Negative scenarios**: Error handling and edge cases
- **Safety scenarios**: Black screen prevention, rollback verification
- **Performance scenarios**: Target durations documented

Physical machine testing scripts:

- `physical_machine_safety_test.bat` - Production validation
- `emergency_recovery.bat` - Recovery verification
- `test_all_fixes.bat` - Comprehensive functionality

---

## Dependencies

### Build-time
- Windows Driver Kit (WDK) 10.0.22621+
- Visual Studio 2022 (MSVC v143)
- CMake 3.20+

### Runtime
- Windows 10 2004+ / Windows 11
- WDDM 2.x display driver model
- Administrator privileges (for install/uninstall only)

---

## Known Limitations

### VirtualBox Environment

Virtual displays are recognized but may not fully integrate with Windows Display Settings UI due to VM display management layer. **Recommendation**: Test on physical hardware for production validation.

### Broker Service

Session management and advanced recovery features require VddSvc.exe broker service, planned for v1.1.0 release. Current workarounds available via `RecoverOrphanedState()`.

---

## Contributing

When making changes to specifications:

1. Read `AGENTS.md` for OpenSpec workflow
2. Create a change proposal in `changes/<change-id>/`
3. Write spec deltas with `## ADDED|MODIFIED|REMOVED Requirements`
4. Validate with `openspec validate <change-id> --strict`
5. Request approval before implementation

---

## Documentation Links

- **Design Document**: `../UPDATED_DESIGN_DOCUMENT.md`
- **API Header**: `../vddsdk.h`
- **CLI Reference**: Run `../build/bin/Release/vddctl.exe --help`
- **Safety Guide**: `../SAFETY_MECHANISMS.md`
- **Implementation Notes**: `../CODE_IMPLEMENTATION_DOCUMENTATION.md`

---

## Contact & Support

For issues or questions about these specifications:

1. Check `project.md` troubleshooting section
2. Run `openspec validate` to check spec integrity
3. Review relevant spec file in `specs/`
4. File issue with reproduction steps and `vddctl status` output

---

**Specification Status**: ✅ Production Ready (5 specs) + 🔄 1 Change Proposal  
**Implementation Status**: v1.0.0 Core Complete, v1.1.0 Session Management Pending  
**Last Validation**: 2025-11-06  
- Specs: 5 passed, 0 failed  
- Changes: 1 valid (add-session-management)

