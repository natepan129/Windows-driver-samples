# VDD SDK OpenSpec - Unified Specification

**Version**: 1.0.0  
**Status**: Production Ready  
**Last Updated**: 2025-11-06

---

## Purpose

This is the **single source of truth** for the Virtual Display Driver (VDD) SDK project. All code changes MUST align with this specification.

**For AI Assistants (Cursor, Copilot, etc.)**:
- Read this file FIRST before making any changes
- Follow all `SHALL`/`MUST` requirements strictly
- Check `## Anti-Patterns` before writing code
- Reference specific sections when proposing changes

---

## Quick Navigation

| Section | Location | Purpose |
|---------|----------|---------|
| **Project Overview** | [`openspec/project.md`](openspec/project.md) | Components, conventions, architecture |
| **Capabilities** | [`openspec/specs/`](openspec/specs/) | Detailed requirements per capability |
| **Active Changes** | [`openspec/changes/`](openspec/changes/) | Pending proposals and tasks |
| **Implementation Guide** | See below | Code structure and file locations |

---

## Goals

1. **Zero Black Screens**: No display corruption on physical machines
2. **Production-Ready**: Reliable driver lifecycle on Windows 10/11 x64
3. **Fault-Tolerant**: Automatic recovery from crashes and orphaned state
4. **Developer-Friendly**: Simple C++ API + CLI tool for any language
5. **Spec-Driven**: All changes must have OpenSpec proposals first

---

## Non-Goals

- ❌ Cross-platform support (Windows-only by design)
- ❌ Windows 7/8 support (requires Windows 10 2004+ / Win11)
- ❌ Hot-plug event handling (defer to v2.0)
- ❌ Hardware acceleration of frame encoding (UMDF limitation)
- ❌ Multi-user concurrent sessions (single-session design)

---

## Architecture

```
┌─────────────────────────────────────────────────────────┐
│  Applications (Unity, Unreal, OBS, Parsec, etc.)       │
└────────────────┬────────────────────────────────────────┘
                 │ calls
                 ▼
┌─────────────────────────────────────────────────────────┐
│  vddctl.exe (CLI Tool)                                   │
│  - Simple command interface                              │
│  - JSON output for scripting                             │
└────────────────┬────────────────────────────────────────┘
                 │ links
                 ▼
┌─────────────────────────────────────────────────────────┐
│  vddsdk.dll (C++ SDK)                                    │
│  - Driver Lifecycle: Install/Uninstall/Query           │
│  - Runtime Control: Activate/Deactivate                │
│  - Display Config: SetMode/SetLocation/SetPrimary      │
│  - Query: EnumerateAdapters/Modes/Outputs             │
│  - Session Management: Leasing + Heartbeat (v1.1)     │
│  - Recovery: Orphaned state cleanup                    │
└────────────────┬────────────────────────────────────────┘
                 │ uses SetupAPI, DisplayConfig, DXGI
                 ▼
┌─────────────────────────────────────────────────────────┐
│  Windows Display Subsystem                              │
│  - SetupAPI: Device installation/enumeration           │
│  - SetDisplayConfig: Topology management                │
│  - ChangeDisplaySettingsExW: Mode changes              │
│  - DXGI: Output enumeration                             │
└────────────────┬────────────────────────────────────────┘
                 │ loads
                 ▼
┌─────────────────────────────────────────────────────────┐
│  IddSampleDriver.dll (UMDF Driver)                      │
│  - IddCx framework (Indirect Display)                   │
│  - 3 virtual monitors with custom EDID                  │
│  - Swapchain processing loop                            │
│  - Loaded by WUDFHost.exe                               │
└─────────────────────────────────────────────────────────┘
```

---

## Core Capabilities

Detailed specs: [`openspec/specs/`](openspec/specs/)

### 1. Driver Lifecycle ([spec](openspec/specs/vdd-driver-lifecycle/spec.md))

**SHALL provide**:
- ✅ Driver installation with automatic rollback on failure
- ✅ Complete driver uninstallation with Driver Store cleanup
- ✅ Multi-source detection (Device Manager, Driver Store, Services)
- ✅ Driver version query

**Implementation**: `vddsdk.cpp` lines 585-995

---

### 2. Runtime Control ([spec](openspec/specs/vdd-runtime-control/spec.md))

**SHALL provide**:
- ✅ Virtual display activation with **primary display protection**
- ✅ Graceful deactivation with window migration
- ✅ Activation state query (IsActive, GetActiveDisplayCount)
- ✅ Multi-layer safety for physical machines

**Implementation**: `vddsdk.cpp` lines 1113-1488

---

### 3. Display Configuration ([spec](openspec/specs/vdd-display-configuration/spec.md))

**SHALL provide**:
- ✅ Display mode setting (resolution, refresh rate)
- ✅ Display positioning (X, Y coordinates)
- ✅ Primary display designation
- ✅ Topology preservation

**Implementation**: `vddsdk.cpp` lines 1490-1874

---

### 4. Query ([spec](openspec/specs/vdd-query/spec.md))

**SHALL provide**:
- ✅ Adapter enumeration (physical + virtual)
- ✅ Mode enumeration per output
- ✅ DXGI output lookup by name
- ✅ Device detection

**Implementation**: `vddsdk.cpp` lines 1957-2048

---

### 5. Recovery ([spec](openspec/specs/vdd-recovery/spec.md))

**SHALL provide**:
- ✅ Orphaned state detection and cleanup
- ✅ Driver health monitoring
- ✅ Automatic recovery workflows

**Implementation**: Planned for v1.1

---

### 6. Session Management ([spec](openspec/changes/add-session-management/))

**SHALL provide** (v1.1):
- ⏳ Fault-tolerant sessions with leasing
- ⏳ Heartbeat monitoring (1-2s intervals)
- ⏳ Automatic cleanup on application crash
- ⏳ Broker service (VddSvc.exe)

**Status**: Proposal stage

---

## Constraints

### Technical Constraints

1. **MUST require Windows 10 2004+ or Windows 11** (no Win7/8 support)
2. **MUST require x64 architecture** (no x86/ARM)
3. **MUST require Administrator privileges** for install/uninstall
4. **MUST use UMDF driver model** (user-mode only)
5. **MUST support up to 10 virtual displays** (practical limit)

### Safety Constraints

6. **MUST protect primary display** during all topology changes
7. **MUST validate display config** before applying (SDC_VALIDATE)
8. **MUST implement rollback** on installation failure
9. **MUST poll device readiness** instead of fixed delays
10. **MUST provide emergency recovery** scripts for black screen

### API Constraints

11. **MUST return Status enum** from all operations
12. **MUST use Hardware ID-based** device identification (not strings)
13. **MUST preserve DEVMODE fields** when changing modes
14. **MUST use stable ContainerIds** in driver (deterministic GUIDs)
15. **MUST validate EDID checksums** before exposing to OS

---

## Anti-Patterns (MUST NOT / NEVER)

### ❌ Display Configuration

**NEVER** blindly replay `QueryDisplayConfig` results:
```cpp
// ❌ BAD: Causes topology corruption
QueryDisplayConfig(QDC_ONLY_ACTIVE_PATHS, &paths, &modes, ...);
SetDisplayConfig(paths, modes, SDC_APPLY | SDC_USE_SUPPLIED_DISPLAY_CONFIG);
// Windows may auto-adjust during replay → black screen
```

**INSTEAD** use explicit topology strategy:
```cpp
// ✅ GOOD: Explicit topology control
SetDisplayConfig(paths, modes, SDC_APPLY | SDC_TOPOLOGY_EXTEND | SDC_ALLOW_CHANGES);
```

---

**NEVER** use string-based device matching:
```cpp
// ❌ BAD: Breaks on non-English Windows
if (deviceString.find(L"IddSampleDriver") != std::wstring::npos)

// ✅ GOOD: Hardware ID-based
SetupDiGetDeviceRegistryPropertyW(hDev, &dev, SPDRP_HARDWAREID, ...);
for (wchar_t* p = hwid; *p; p += wcslen(p) + 1) {
    if (_wcsicmp(p, L"ROOT\\IddSampleDriver") == 0) { ... }
}
```

---

**NEVER** set primary display by position alone:
```cpp
// ❌ BAD: Doesn't actually change primary
sourceMode.position.x = 0;
sourceMode.position.y = 0;
SetDisplayConfig(...);  // Only moves display, doesn't set primary flag

// ✅ GOOD: Use CDS_SET_PRIMARY
ChangeDisplaySettingsExW(device, NULL, NULL, CDS_SET_PRIMARY | CDS_UPDATEREGISTRY | CDS_NORESET, NULL);
ChangeDisplaySettingsExW(NULL, NULL, NULL, 0, NULL);  // Commit
```

---

**NEVER** use runtime GUIDs for ContainerIds:
```cpp
// ❌ BAD (Driver.cpp): Layout resets every reboot
CoCreateGuid(&MonitorContainerId);

// ✅ GOOD: Deterministic GUID
MonitorContainerId = GenerateStableContainerId(ConnectorIndex);
```

---

**NEVER** ignore EDID checksums:
```cpp
// ❌ BAD: OS ignores invalid EDID → limited resolutions
static const BYTE edid[128] = { ..., 0xDF };  // Wrong checksum

// ✅ GOOD: Validate and correct
BYTE checksum = CalculateEdidChecksum(edid, 127);
edid[127] = checksum;
```

---

### ❌ Error Handling

**NEVER** swallow errors silently:
```cpp
// ❌ BAD
if (!SetupDiCallClassInstaller(...)) {
    // Silent failure
}

// ✅ GOOD
if (!SetupDiCallClassInstaller(...)) {
    DWORD err = GetLastError();
    printf("[VDD] ERROR: DIF_INSTALLDEVICE failed, error=%d\n", err);
    SetLastError("Installation failed: " + std::to_string(err));
    return Status::DriverError;
}
```

---

**NEVER** use hardcoded delays:
```cpp
// ❌ BAD: Too short on slow machines, too long on fast ones
Sleep(500);

// ✅ GOOD: Poll until ready
for (int i = 0; i < 30; i++) {  // Max 3s
    if (CM_Get_DevNode_Status(&status, &problem, devInst, 0) == CR_SUCCESS &&
        (status & DN_STARTED) && !(status & DN_HAS_PROBLEM)) {
        break;  // Device ready
    }
    Sleep(100);
}
```

---

### ❌ Resource Management

**NEVER** forget to cleanup on failure:
```cpp
// ❌ BAD: Leaks hDevInfo on error
HDEVINFO hDevInfo = SetupDiCreateDeviceInfoList(...);
if (!SetupDiCreateDeviceInfoW(...)) {
    return Status::DriverError;  // LEAK!
}

// ✅ GOOD: RAII or explicit cleanup
HDEVINFO hDevInfo = SetupDiCreateDeviceInfoList(...);
if (!SetupDiCreateDeviceInfoW(...)) {
    SetupDiDestroyDeviceInfoList(hDevInfo);
    return Status::DriverError;
}
```

---

## Implementation Files

| Capability | Primary File | Lines | Key Functions |
|------------|--------------|-------|---------------|
| **Driver Lifecycle** | `vddsdk.cpp` | 585-995 | InstallDriver, UninstallDriver |
| **Runtime Control** | `vddsdk.cpp` | 1113-1488 | Activate, Deactivate, IsActive |
| **Display Config** | `vddsdk.cpp` | 1490-1874 | SetMode, SetLocation, SetPrimary |
| **Query** | `vddsdk.cpp` | 1957-2048 | EnumerateAdapters, EnumerateModes |
| **CLI Tool** | `vddctl.cpp` | 1-503 | cmdInstall, cmdActivate, cmdSetMode |
| **Driver Core** | `Driver.cpp` | 1-907 | IddSampleAdapterInitFinished, FinishInit |

---

## Testing Requirements

All changes MUST include:

1. **Unit tests** for affected functions (mock SetupAPI/DisplayConfig)
2. **Integration tests** on physical hardware (5+ machines minimum)
3. **Regression tests** - existing test scripts must still pass
4. **Black screen tests** - 100 activate/deactivate cycles without corruption

**Test Configurations**:
- Single monitor desktop
- Dual monitor extended
- Laptop (internal + external)
- Triple monitor setup
- Mixed resolutions (4K + 1080p)
- 59.94Hz and 60Hz panels

---

## Change Workflow

**All changes MUST follow OpenSpec workflow**:

1. **Create proposal** in `openspec/changes/[name]/`
   - `proposal.md` - Why, what, impact
   - `tasks.md` - Implementation checklist
   - `design.md` - Technical decisions
   - `specs/[capability]/spec.md` - ADDED/MODIFIED/REMOVED requirements

2. **Validate proposal**: `openspec validate [name] --strict`

3. **Get approval** before implementing

4. **Implement** according to tasks.md

5. **Test** according to Testing Requirements

6. **Archive** after deployment: `openspec archive [name] --yes`

---

## Current Active Changes

| Change | Status | Target | Description |
|--------|--------|--------|-------------|
| [`add-session-management`](openspec/changes/add-session-management/) | ✅ Validated | v1.1 | Broker service, leasing, heartbeat |
| [`fix-display-configuration-issues`](openspec/changes/fix-display-configuration-issues/) | ✅ Validated | v1.0.1 | Fix 21 critical bugs (black screens, ContainerId, EDID) |

---

## References

- **Project Conventions**: [`openspec/project.md`](openspec/project.md)
- **Capability Specs**: [`openspec/specs/`](openspec/specs/)
- **Change Proposals**: [`openspec/changes/`](openspec/changes/)
- **Windows Display APIs**: https://learn.microsoft.com/en-us/windows/win32/gdi/display-devices
- **SetupAPI Reference**: https://learn.microsoft.com/en-us/windows-hardware/drivers/install/setupapi
- **IddCx Documentation**: https://learn.microsoft.com/en-us/windows-hardware/drivers/display/indirect-display-driver-model-overview

---

**For Cursor AI / Copilot**: Always reference specific sections of this spec when making changes. Use `## Anti-Patterns` to avoid common mistakes. Follow `## Constraints` strictly.

