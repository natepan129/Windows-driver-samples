# Change: Add Fault-Tolerant Session Management

## Why

Applications using virtual displays can crash or terminate unexpectedly, leaving orphaned virtual displays active. This causes:
- Black screens if VDD was set as primary
- Cluttered display topology that confuses users
- Need for manual cleanup via Device Manager or emergency scripts

A broker-based session management system with leasing and heartbeat monitoring will automatically detect crashed applications and restore the original display configuration.

## What Changes

- **Add VddSvc.exe broker service** - Background Windows service that manages display sessions
- **Implement lease-based ownership** - Applications acquire leases to "own" virtual displays
- **Add heartbeat monitoring** - Applications send periodic heartbeats (1-2s intervals)
- **Automatic cleanup on timeout** - Broker detects missed heartbeats (>5s) and deactivates displays
- **Topology backup and restore** - Broker saves original configuration and restores on cleanup
- **IPC communication layer** - Named pipe protocol between SDK and broker

## Impact

### Affected Specs
- **NEW**: `vdd-session-management` - Complete new capability
- **MODIFIED**: `vdd-recovery` - Integration with broker for topology restore

### Affected Code
- **NEW**: `VddSvc/` - New broker service project
  - `VddSvc.exe` - Service executable
  - `BrokerService.cpp/.h` - Service implementation
  - `LeaseManager.cpp/.h` - Lease tracking and heartbeat monitoring
  - `TopologyBackup.cpp/.h` - Display configuration persistence
  - `IpcServer.cpp/.h` - Named pipe communication
- **MODIFIED**: `vddsdk.cpp` - Add session management API implementations
  - `BeginSession()`, `ActivateLeased()`, `Heartbeat()`, `EndSession()`
  - IPC client for broker communication
- **MODIFIED**: `vddctl.cpp` - Add CLI commands
  - `vddctl begin-session`, `vddctl heartbeat`, `vddctl end-session`

### Dependencies
- Windows Service Control Manager APIs
- Named pipe IPC (`CreateNamedPipe`, `ConnectNamedPipe`)
- Registry for persistent topology storage
- JSON serialization library (e.g., nlohmann/json) for IPC messages

### Breaking Changes
- None - Session management is optional; existing APIs continue to work without broker
- Applications can choose to use `Activate()` (simple) or `ActivateLeased()` (fault-tolerant)

## Risks

### Technical Risks
- **Broker service reliability**: Service must be rock-solid to avoid becoming single point of failure
  - Mitigation: Automatic service restart on crash, comprehensive error handling
- **Heartbeat overhead**: 1-2s interval IPC calls could impact performance
  - Mitigation: Lightweight protocol (<100 bytes), async design, <10ms latency target
- **Race conditions**: Multiple applications may try to claim same displays
  - Mitigation: Mutex-based lease allocation, clear ownership model

### Deployment Risks
- **Service installation**: Requires admin privileges, may fail on restricted systems
  - Mitigation: Graceful degradation (session APIs return `Status::DriverError` if broker unavailable)
- **Firewall/security**: Named pipes may be blocked by security software
  - Mitigation: Use local-only pipes (`\\.\pipe\VddBroker`), document security requirements

## Timeline

- **Week 1-2**: Broker service scaffold + IPC protocol design
- **Week 3-4**: Lease management + heartbeat monitoring
- **Week 5-6**: SDK integration (`BeginSession`, `ActivateLeased`, etc.)
- **Week 7**: CLI tool integration + testing
- **Week 8**: Documentation + deployment testing

**Target Release**: v1.1.0 (Q1 2026)

## Alternatives Considered

### Alternative 1: Process monitoring without IPC
- SDK monitors its own process and registers with Windows
- Con: No centralized cleanup if SDK process hangs (not crashed)
- Con: Cannot handle multi-process scenarios

### Alternative 2: Shared memory instead of named pipes
- Pro: Faster communication
- Con: More complex synchronization
- Con: Harder to debug
- Decision: Named pipes are more robust and debuggable

### Alternative 3: No broker, client-side only
- Applications implement their own recovery via `RecoverOrphanedState()`
- Con: Requires every application to implement recovery logic
- Con: No automatic cleanup on crash
- Decision: Broker provides better user experience

## Success Criteria

- ✅ Broker service installs and runs reliably on Windows 10 2004+
- ✅ Heartbeat latency < 10ms for 99th percentile
- ✅ Automatic cleanup within 5 seconds of application crash
- ✅ Topology restore success rate > 95%
- ✅ Graceful degradation when broker unavailable (fallback to `Activate()`)
- ✅ No memory leaks after 1000+ session cycles
- ✅ Passes physical machine safety tests (no black screens)

