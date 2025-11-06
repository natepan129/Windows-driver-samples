# Implementation Tasks: Add Session Management

## 1. Broker Service Infrastructure

- [ ] 1.1 Create VddSvc project in CMake
  - [ ] Add Windows Service template code
  - [ ] Configure service installation via `sc create VddSvc`
  - [ ] Add service control handlers (Start, Stop, Pause)
- [ ] 1.2 Implement service lifecycle
  - [ ] `ServiceMain()` entry point
  - [ ] `ServiceCtrlHandler()` for control codes
  - [ ] Graceful shutdown on stop signal
- [ ] 1.3 Add logging infrastructure
  - [ ] Event log registration
  - [ ] Debug output for development
  - [ ] Log rotation for production

## 2. IPC Protocol Design

- [ ] 2.1 Define message format (JSON)
  - [ ] Request: `{ "op": "BEGIN_SESSION|HEARTBEAT|...", "data": {...} }`
  - [ ] Response: `{ "status": "Ok|Error|...", "leaseId": 123, "data": {...} }`
- [ ] 2.2 Implement IPC server (broker side)
  - [ ] Create named pipe `\\.\pipe\VddBroker`
  - [ ] Handle multiple simultaneous client connections
  - [ ] Thread pool for connection handling
- [ ] 2.3 Implement IPC client (SDK side)
  - [ ] Connect to named pipe
  - [ ] Send/receive with timeout
  - [ ] Reconnect logic on disconnect
- [ ] 2.4 Add protocol tests
  - [ ] Unit tests for message serialization
  - [ ] Integration tests for full request/response cycle

## 3. Lease Management

- [ ] 3.1 Implement `LeaseManager` class
  - [ ] Allocate unique lease IDs (64-bit random)
  - [ ] Track lease ownership (process ID, lease ID, display indices)
  - [ ] Support shared ownership (multiple leases per display)
- [ ] 3.2 Add lease operations
  - [ ] `AllocateLease(processId) -> leaseId`
  - [ ] `AssociateDisplays(leaseId, displayIndices)`
  - [ ] `ReleaseLease(leaseId)`
  - [ ] `QueryLeaseState(leaseId) -> { active, displays, lastHeartbeat }`
- [ ] 3.3 Implement lease persistence (optional)
  - [ ] Save to registry on allocation
  - [ ] Restore on service restart

## 4. Heartbeat Monitoring

- [ ] 4.1 Implement heartbeat tracking
  - [ ] Store `lastHeartbeat` timestamp per lease
  - [ ] Update on `HEARTBEAT` message
- [ ] 4.2 Create monitoring thread
  - [ ] Check all leases every 1 second
  - [ ] Detect leases with `lastHeartbeat > 5 seconds ago`
  - [ ] Trigger cleanup for timed-out leases
- [ ] 4.3 Add configurable thresholds
  - [ ] Heartbeat interval (default 1.5s)
  - [ ] Timeout threshold (default 5s)
  - [ ] Read from registry or config file

## 5. Topology Backup and Restore

- [ ] 5.1 Implement `TopologyBackup` class
  - [ ] Capture current display configuration via `QueryDisplayConfig`
  - [ ] Store: primary index, display rects, modes, paths
  - [ ] Serialize to JSON for registry storage
- [ ] 5.2 Add backup operations
  - [ ] `CaptureTopology() -> TopologySnapshot`
  - [ ] `SaveToRegistry(leaseId, snapshot)`
  - [ ] `LoadFromRegistry(leaseId) -> TopologySnapshot`
- [ ] 5.3 Implement restore logic
  - [ ] Apply stored configuration via `SetDisplayConfig`
  - [ ] Deactivate virtual displays first
  - [ ] Restore primary display designation
  - [ ] Fallback to safe mode if restore fails

## 6. SDK Integration

- [ ] 6.1 Add session management APIs to `vddsdk.cpp`
  - [ ] `BeginSession(LeaseHandle& out)`
  - [ ] `ActivateLeased(ActivateOptions, LeaseHandle)`
  - [ ] `Heartbeat(LeaseHandle)`
  - [ ] `EndSession(LeaseHandle)`
  - [ ] `GetState(...)` (query broker state)
- [ ] 6.2 Implement IPC client wrapper
  - [ ] `BrokerClient` class for communication
  - [ ] Connection pooling/reuse
  - [ ] Error handling and retry logic
- [ ] 6.3 Add background heartbeat helper (optional)
  - [ ] `AutoHeartbeat` RAII class that sends heartbeats in background thread
  - [ ] Destructor calls `EndSession()`

## 7. CLI Tool Integration

- [ ] 7.1 Add new `vddctl` commands
  - [ ] `vddctl begin-session` -> outputs lease ID
  - [ ] `vddctl heartbeat --lease <id>`
  - [ ] `vddctl end-session --lease <id>`
  - [ ] `vddctl query-leases` (admin only)
- [ ] 7.2 Update `activate` command
  - [ ] Add `--leased` flag to use `ActivateLeased`
  - [ ] Add `--auto-restore` flag
  - [ ] Add `--lease <id>` to associate with existing lease

## 8. Testing

- [ ] 8.1 Unit tests
  - [ ] Lease allocation and release
  - [ ] Heartbeat timeout detection
  - [ ] Topology serialization/deserialization
  - [ ] IPC message parsing
- [ ] 8.2 Integration tests
  - [ ] Full session lifecycle (begin -> activate -> heartbeat -> end)
  - [ ] Crash simulation (stop heartbeats, verify cleanup)
  - [ ] Multi-process leasing (shared displays)
  - [ ] Broker restart recovery
- [ ] 8.3 Physical machine tests
  - [ ] Kill application mid-session, verify automatic restore
  - [ ] Verify no black screens during cleanup
  - [ ] Test on Windows 10 2004+ and Windows 11
- [ ] 8.4 Performance tests
  - [ ] Heartbeat latency benchmark (target <10ms)
  - [ ] 1000+ session cycles without memory leak
  - [ ] Concurrent lease stress test (100+ leases)

## 9. Documentation

- [ ] 9.1 Update API documentation
  - [ ] Add session management examples to `vddsdk.h`
  - [ ] Update README with broker setup instructions
- [ ] 9.2 Create broker deployment guide
  - [ ] Service installation steps
  - [ ] Configuration options
  - [ ] Troubleshooting common issues
- [ ] 9.3 Update OpenSpec
  - [ ] Archive this change to `specs/vdd-session-management/`
  - [ ] Update `project.md` roadmap

## 10. Deployment

- [ ] 10.1 Create installer scripts
  - [ ] `install_broker.bat` (admin required)
  - [ ] `uninstall_broker.bat`
  - [ ] Automatic service start configuration
- [ ] 10.2 Add to build system
  - [ ] CMake target for VddSvc.exe
  - [ ] Copy broker to `build/bin/Release/`
- [ ] 10.3 Create deployment package
  - [ ] Bundle SDK DLL + CLI tool + broker service
  - [ ] Include setup instructions

## Acceptance Criteria

All tasks must be complete with:
- ✅ Code compiles without warnings
- ✅ All unit tests pass
- ✅ Integration tests pass on physical hardware
- ✅ Performance targets met (heartbeat <10ms, cleanup <5s)
- ✅ Documentation updated
- ✅ Peer review approved



