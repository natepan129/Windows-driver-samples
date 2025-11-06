# Design: Fault-Tolerant Session Management

## Context

Current implementation supports basic virtual display activation/deactivation, but lacks crash recovery. If an application using VDD crashes:
- Virtual displays remain active (orphaned state)
- May cause black screens if VDD was set as primary
- User must manually run recovery scripts or reboot

Design goal: Automatic detection and cleanup of crashed applications without user intervention.

## Goals

- **Automatic crash detection**: Detect when applications terminate unexpectedly
- **Fast cleanup**: Restore displays within 5 seconds of crash
- **Topology preservation**: Save and restore original configuration
- **Zero user interaction**: No manual cleanup required
- **Backward compatible**: Existing `Activate()` API continues to work

## Non-Goals

- **Cross-machine sessions**: Remote applications over network (future v2.0)
- **Distributed broker**: Multiple brokers for redundancy
- **GUI configuration**: CLI/API only for v1.1
- **Historical metrics**: No telemetry or analytics (add later if needed)

---

## Architecture

### Component Diagram

```
┌─────────────┐                    ┌──────────────┐
│ Application │                    │   VddSvc.exe │
│             │                    │   (Broker)   │
│  vddsdk.dll │◄──────IPC─────────►│              │
│             │   Named Pipe       │              │
└─────────────┘                    └──────┬───────┘
       │                                  │
       │ SetupAPI                         │ SetupAPI
       ▼                                  ▼
┌─────────────────────────────────────────────────┐
│         Windows Display Stack                    │
│  (IddSampleDriver.dll via WUDFHost.exe)         │
└──────────────────────────────────────────────────┘
```

### Data Flow

**1. Session Creation**
```
App: BeginSession()
  → SDK: Connect to \\.\pipe\VddBroker
  → Broker: AllocateLease(processId)
  → Broker: Return leaseId=12345
  ← SDK: Return LeaseHandle{id:12345}
```

**2. Leased Activation**
```
App: ActivateLeased({desc, count, autoRestore=true}, lease)
  → SDK: ACTIVATE_LEASED message to broker
  → Broker: CaptureTopology() → save to registry
  → Broker: AssociateDisplays(leaseId, [0,1,2])
  → SDK: Call Activate() internally
  ← Device enabled, appears in Windows
```

**3. Heartbeat Loop**
```
App: while(running) { Heartbeat(lease); sleep(1.5s); }
  → SDK: HEARTBEAT message with leaseId
  → Broker: Update lease.lastHeartbeat = now()
```

**4. Crash Detection**
```
Broker monitoring thread (every 1s):
  for each lease:
    if (now() - lease.lastHeartbeat > 5s):
      CleanupLease(lease)
        → Deactivate displays
        → LoadTopology(leaseId) from registry
        → RestoreTopology() via SetDisplayConfig
        → ReleaseLease(leaseId)
```

---

## Decisions

### Decision 1: Named Pipes vs Shared Memory

**Options**:
- A) Named pipes (`CreateNamedPipe`, `ConnectNamedPipe`)
- B) Shared memory + mutexes
- C) Local TCP sockets

**Choice**: Named pipes (A)

**Rationale**:
- Built-in access control (DACL)
- Easier to debug (can inspect with tools)
- Simpler error handling
- Adequate performance (<10ms for small messages)

**Trade-offs**:
- Slightly slower than shared memory (~2-5ms overhead)
- Acceptable for 1-2s heartbeat intervals

---

### Decision 2: Heartbeat Interval

**Options**:
- A) 1-2 second interval (configurable)
- B) 100ms interval (low latency)
- C) 10 second interval (low overhead)

**Choice**: 1-2 seconds (A), timeout at 5 seconds

**Rationale**:
- Balance between responsiveness and overhead
- 5s timeout allows for process scheduling delays
- Avoid false positives from GC pauses or CPU contention

**Trade-offs**:
- Cleanup may take up to 5s after crash
- Acceptable for user experience (Win+P is instant if needed)

---

### Decision 3: Topology Storage

**Options**:
- A) Registry (`HKLM\SOFTWARE\VDD\Topology\{LeaseId}`)
- B) JSON file (`%ProgramData%\VDD\topology_{leaseId}.json`)
- C) In-memory only

**Choice**: Registry (A) with in-memory cache

**Rationale**:
- Persistent across broker restarts
- Windows standard for service configuration
- Easy to inspect/debug with `regedit`

**Trade-offs**:
- Registry I/O is slower (~10-20ms)
- Mitigated by in-memory cache for hot path

---

### Decision 4: Lease Ownership Model

**Options**:
- A) Exclusive ownership (one lease per display)
- B) Shared ownership (multiple leases, reference counted)
- C) Hybrid (exclusive by default, shared opt-in)

**Choice**: Shared ownership (B) for flexibility

**Rationale**:
- Enables multi-process scenarios (e.g., compositor + app)
- Deactivation only happens when all leases released
- More complex but more powerful

**Implementation**:
```cpp
struct Lease {
    uint64_t id;
    DWORD processId;
    std::vector<uint32_t> displayIndices;
    time_point lastHeartbeat;
};

// Track reference count per display
std::map<uint32_t, std::set<uint64_t>> displayOwners;
```

---

## Implementation Details

### IPC Message Protocol

**JSON Schema** (example):

```json
// Request
{
  "op": "BEGIN_SESSION",
  "data": {}
}

// Response
{
  "status": "Ok",
  "leaseId": 12345678901234567,
  "data": {}
}

// Heartbeat
{
  "op": "HEARTBEAT",
  "data": { "leaseId": 12345678901234567 }
}

// Activate Leased
{
  "op": "ACTIVATE_LEASED",
  "data": {
    "leaseId": 12345678901234567,
    "desc": { "name": "VDD XR", "mode": {...} },
    "count": 1,
    "autoRestore": true
  }
}
```

### Topology Snapshot Format

**Registry Key**: `HKLM\SOFTWARE\VDD\Topology\{LeaseId}`

**Values**:
- `Timestamp` (REG_SZ): ISO 8601 timestamp
- `PrimaryIndex` (REG_DWORD): Original primary display
- `PathsJson` (REG_SZ): JSON-serialized `DISPLAYCONFIG_PATH_INFO[]`
- `ModesJson` (REG_SZ): JSON-serialized `DISPLAYCONFIG_MODE_INFO[]`

**Example JSON**:
```json
{
  "timestamp": "2025-11-06T12:00:00Z",
  "primaryIndex": 0,
  "displays": [
    {
      "deviceName": "\\\\.\\DISPLAY1",
      "rect": { "x": 0, "y": 0, "width": 1920, "height": 1080 },
      "mode": { "width": 1920, "height": 1080, "refresh": 60 }
    }
  ]
}
```

### Heartbeat Monitoring Thread

**Pseudocode**:
```cpp
void MonitoringThread() {
    while (serviceRunning) {
        auto now = steady_clock::now();
        
        std::lock_guard lock(leasesMutex);
        for (auto& [leaseId, lease] : leases) {
            auto elapsed = duration_cast<seconds>(now - lease.lastHeartbeat);
            
            if (elapsed.count() > TIMEOUT_THRESHOLD) {
                LOG("Lease %llu timed out, cleaning up", leaseId);
                CleanupLease(lease);
                leases.erase(leaseId);
            }
        }
        
        Sleep(1000); // Check every second
    }
}
```

---

## Risks and Mitigations

### Risk 1: Broker service becomes single point of failure

**Mitigation**:
- Broker runs as Windows service with automatic restart policy
- SDK gracefully degrades if broker unavailable (falls back to `Activate()`)
- Comprehensive error handling and logging

### Risk 2: False positives from GC pauses or system suspend

**Mitigation**:
- 5-second timeout provides buffer for typical GC pauses (<2s)
- System suspend detection: pause monitoring during `WM_POWERBROADCAST`
- Configurable timeout via registry

### Risk 3: Topology restore fails on complex multi-monitor setups

**Mitigation**:
- Validate topology before saving (reject invalid configs)
- Safe mode fallback: `DisplaySwitch.exe /internal` if restore fails
- Log detailed error information for troubleshooting

### Risk 4: Race condition between multiple applications

**Mitigation**:
- Mutex-protected lease allocation
- Atomic operations for display ownership tracking
- Clear ownership model (shared reference counting)

---

## Performance Targets

| Metric | Target | Measurement Method |
|--------|--------|-------------------|
| Heartbeat latency | <10ms (99th percentile) | IPC round-trip time |
| Cleanup time after crash | <5s | Time from missed heartbeat to display restore |
| Service startup time | <2s | Time from `sc start` to accepting connections |
| Memory per lease | <1KB | Lease struct size + overhead |
| Concurrent leases supported | 100+ | Stress test with many clients |

---

## Security Considerations

### Named Pipe Security

**DACL (Discretionary Access Control List)**:
- Allow: `LOCAL_SYSTEM`, `BUILTIN\Administrators`
- Allow: `Authenticated Users` (read/write)
- Deny: `Everyone` without authentication

**Implementation**:
```cpp
SECURITY_DESCRIPTOR sd;
InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
SetSecurityDescriptorDacl(&sd, TRUE, dacl, FALSE);

SECURITY_ATTRIBUTES sa = { sizeof(sa), &sd, FALSE };
CreateNamedPipeW(L"\\\\.\\pipe\\VddBroker", ..., &sa);
```

### Lease Validation

- Verify process ID still exists before cleanup
- Reject leases from terminated processes
- Prevent lease ID guessing (64-bit random, cryptographic quality)

---

## Open Questions

1. **Should broker support remote connections (TCP)?**
   - Current: Local-only named pipes
   - Future: Could add optional TCP for remote desktop scenarios

2. **Should heartbeat be automatic or manual?**
   - Current: Manual `Heartbeat()` calls
   - Alternative: SDK spawns background thread automatically
   - Decision: Manual for now, add auto-helper class in v1.2

3. **How to handle broker updates?**
   - Seamless service restart without dropping leases?
   - Or require all applications to reconnect?
   - Decision: Defer to v1.2 (leases lost on broker restart for now)

---

## Migration Plan

### Phase 1: Add broker (v1.1)
- Release broker as separate service
- Session management APIs available but optional
- Existing `Activate()` continues to work

### Phase 2: Encourage adoption (v1.2)
- Add auto-heartbeat helper class
- Document benefits in README
- Show examples in samples/

### Phase 3: Deprecate non-leased activation (v2.0)
- Make leased activation the default
- `Activate()` becomes wrapper around `ActivateLeased()`
- Broker becomes mandatory component

---

## References

- **Design Document**: Original VDD SDK design (provided by user)
- **Windows Service**: https://learn.microsoft.com/en-us/windows/win32/services/services
- **Named Pipes**: https://learn.microsoft.com/en-us/windows/win32/ipc/named-pipes
- **DisplayConfig API**: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setdisplayconfig



