# VDD Session Management

## Purpose

This capability provides fault-tolerant session management with leasing, heartbeat monitoring, and automatic recovery for applications using virtual displays. It prevents orphaned virtual displays when applications crash or terminate unexpectedly.

---

## ADDED Requirements

### Requirement: Session Creation

The system SHALL provide a function to create a control session with a unique lease handle.

#### Scenario: Create new session
- **WHEN** `BeginSession()` is called
- **THEN** a unique lease handle is allocated by the broker
- **AND** an IPC channel (named pipe/ALPC) is established
- **AND** the lease is registered with the application process ID
- **AND** the function returns `Status::Ok` with a valid `LeaseHandle`

#### Scenario: Multiple sessions per process
- **WHEN** `BeginSession()` is called multiple times in the same process
- **THEN** each call returns a unique lease handle
- **AND** each lease is independently tracked and managed

#### Scenario: Session creation when broker unavailable
- **WHEN** `BeginSession()` is called but the VDD broker service is not running
- **THEN** the function attempts to start the service automatically
- **OR** returns `Status::DriverError` if service cannot be started
- **AND** provides guidance via `GetLastError()`

---

### Requirement: Leased Activation

The system SHALL provide a function to activate virtual displays under a session lease with automatic cleanup options.

#### Scenario: Activate with auto-restore on crash
- **WHEN** `ActivateLeased()` is called with `autoRestoreOnCrash = true`
- **THEN** the virtual displays are activated as normal
- **AND** the broker registers the lease as the owner of these displays
- **AND** if heartbeats stop, the broker will clean up and restore original topology

#### Scenario: Activate without auto-restore
- **WHEN** `ActivateLeased()` is called with `autoRestoreOnCrash = false`
- **THEN** the virtual displays remain active even if the application crashes
- **AND** manual cleanup is required via `Deactivate()` or `RecoverOrphanedState()`

#### Scenario: Leased activation with invalid lease
- **WHEN** `ActivateLeased()` is called with an invalid or expired lease handle
- **THEN** the function returns `Status::InvalidArg`
- **AND** no activation occurs

#### Scenario: Idempotent leased activation
- **WHEN** `ActivateLeased()` is called multiple times with the same parameters
- **THEN** subsequent calls detect existing activation
- **AND** return `Status::Ok` without creating duplicate displays

---

### Requirement: Heartbeat Monitoring

The system SHALL provide a function to send periodic heartbeat signals to maintain session validity.

#### Scenario: Normal heartbeat flow
- **WHEN** `Heartbeat()` is called every 1-2 seconds with a valid lease
- **THEN** the broker resets the lease timeout timer
- **AND** the session remains active
- **AND** virtual displays remain enabled

#### Scenario: Missed heartbeats trigger cleanup
- **WHEN** heartbeats are not received for > 5 seconds (configurable threshold)
- **THEN** the broker considers the application crashed or hung
- **AND** automatically deactivates the leased virtual displays
- **AND** restores the original desktop topology (if `autoRestoreOnCrash` was true)

#### Scenario: Heartbeat with invalid lease
- **WHEN** `Heartbeat()` is called with an expired or unknown lease handle
- **THEN** the function returns `Status::NotFound`
- **AND** the application should call `BeginSession()` again

#### Scenario: Heartbeat performance
- **WHEN** `Heartbeat()` is called
- **THEN** the function completes in < 10ms
- **AND** does not block the application main thread

---

### Requirement: Session Termination

The system SHALL provide a function to gracefully release a session lease.

#### Scenario: Normal session end
- **WHEN** `EndSession()` is called with a valid lease
- **THEN** the broker deactivates any displays owned by this lease (if last owner)
- **AND** the lease handle is invalidated
- **AND** the IPC channel is closed
- **AND** the function returns `Status::Ok`

#### Scenario: End session when shared
- **WHEN** `EndSession()` is called but other leases are active for the same displays
- **THEN** the lease is released but displays remain active
- **AND** only the last `EndSession()` call deactivates displays

#### Scenario: End session with invalid lease
- **WHEN** `EndSession()` is called with an already-ended or invalid lease
- **THEN** the function returns `Status::NotFound`
- **AND** the operation is idempotent (no error state)

---

### Requirement: State Query

The system SHALL provide a function to query the current activation state under session management.

#### Scenario: Query active state
- **WHEN** `GetState()` is called
- **THEN** the function returns the number of active virtual displays
- **AND** the primary display index
- **AND** the desktop rectangle for each display
- **AND** which leases own which displays

---

## Implementation Notes

### Broker Service Architecture

The VDD broker (`VddSvc.exe`) is a Windows service that:

- Runs in the background with SYSTEM privileges
- Maintains a registry of active leases (process ID + lease ID mapping)
- Monitors heartbeats via IPC (named pipe `\\.\pipe\VddBroker`)
- Performs automatic cleanup when heartbeats time out

### Heartbeat Timer Implementation

Broker uses a heartbeat timeout mechanism:

```
For each lease:
    lastHeartbeat = current_time
    
On Heartbeat():
    Update lease.lastHeartbeat = current_time
    
Every 1 second:
    For each lease:
        if (current_time - lease.lastHeartbeat > 5 seconds):
            CleanupLease(lease)
```

### Lease Ownership Model

**Single Owner (default)**
- One lease owns the virtual displays
- When lease ends or times out, displays are deactivated

**Shared Ownership (optional)**
- Multiple leases can activate the same displays
- Displays remain active until all leases end
- Useful for multi-process scenarios (e.g., compositor + app)

### Auto-Restore Topology

When `autoRestoreOnCrash = true`, the broker:

1. Captures display topology before activation (primary, positions, modes)
2. Stores in lease metadata
3. On cleanup (timeout or EndSession), restores the saved topology:
   - Deactivate virtual displays
   - Restore original primary display
   - Restore original display positions

### IPC Protocol

Communication between SDK and broker uses Windows named pipes:

**Request Messages:**
- `BEGIN_SESSION`: Returns lease ID
- `ACTIVATE_LEASED`: Lease ID + display params
- `HEARTBEAT`: Lease ID
- `END_SESSION`: Lease ID
- `GET_STATE`: Returns JSON with current state

**Response Format:**
```json
{
  "status": "Ok|Error|NotFound|...",
  "leaseId": 12345,
  "data": { ... }
}
```

---

## Error Handling

| Error | Cause | Resolution |
|-------|-------|------------|
| `Status::DriverError` | Broker service not running | Start VddSvc.exe manually or via `sc start VddSvc` |
| `Status::NotFound` | Lease expired or invalid | Call `BeginSession()` again |
| `Status::Busy` | Another process owns the displays | Wait or forcibly reclaim with admin privileges |
| `Status::Timeout` | Broker unresponsive | Check service status, restart broker |

---

## Performance Characteristics

| Operation | Typical Duration | Notes |
|-----------|------------------|-------|
| BeginSession | 10-50ms | IPC connection establishment |
| ActivateLeased | 500-1000ms | Same as regular Activate |
| Heartbeat | < 10ms | Lightweight IPC message |
| EndSession | 50-200ms | Cleanup + topology restore |
| GetState | 10-50ms | Query broker state |

---

## Security Considerations

- Broker runs as SYSTEM; lease operations are restricted to the owning process
- IPC uses Windows security descriptors to prevent unauthorized access
- Lease IDs are cryptographically random (64-bit) to prevent guessing
- Admin processes can forcibly reclaim leases (emergency override)

---

## Usage Examples

### Example 1: Basic fault-tolerant session

```cpp
LeaseHandle lease;
BeginSession(lease);

ActivateLeased({
    .desc = { "VDD XR", {1920,1080,90,1} },
    .count = 1,
    .autoRestoreOnCrash = true
}, lease);

// Heartbeat loop (in background thread)
while (appRunning) {
    Heartbeat(lease);
    std::this_thread::sleep_for(std::chrono::milliseconds(1500));
}

EndSession(lease);  // Graceful cleanup
```

### Example 2: Shared ownership

```cpp
// Process 1 (compositor)
LeaseHandle compositorLease;
BeginSession(compositorLease);
ActivateLeased({ ... }, compositorLease);

// Process 2 (app)
LeaseHandle appLease;
BeginSession(appLease);
// Reuse existing displays (idempotent activation)
ActivateLeased({ ... }, appLease);

// Both processes send heartbeats
// Displays stay active until both EndSession()
```

### Example 3: Recovery after crash

```cpp
// On application startup, clean up any orphaned state
RecoverOrphanedState();

// Then begin new session
LeaseHandle lease;
BeginSession(lease);
ActivateLeased({ ... }, lease);
```

---

## Dependencies

- VDD broker service (`VddSvc.exe`) must be installed and running
- Windows named pipes for IPC
- Driver must be installed and active

---

## Future Enhancements

### Planned for v1.1

- **Lease Renewal**: Automatic lease extension before timeout
- **Lease Transfer**: Transfer ownership between processes
- **Broker Metrics**: Query broker uptime, active leases, cleanup events
- **Event Notifications**: Callback when lease is forcibly reclaimed

### Planned for v2.0

- **Distributed Sessions**: Support for remote applications over network
- **Persistent Leases**: Survive broker restarts (state saved to disk)
- **Group Leases**: Multiple leases as a single atomic unit

---

## Change History

- **v1.0.0** (2025-11-06): Initial specification based on design document
  - BeginSession for lease creation
  - ActivateLeased with auto-restore option
  - Heartbeat monitoring (1-2s interval, 5s timeout)
  - EndSession for graceful cleanup
  - GetState query support
  - Broker service architecture defined
  - IPC protocol specified

