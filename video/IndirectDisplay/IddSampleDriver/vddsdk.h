/*++

Copyright (c) 2025 VDD SDK Project

Abstract:

    Virtual Display Driver (VDD) SDK - Public API Header
    This header defines the C++ API for managing virtual display drivers on Windows.

Environment:

    User Mode, C++17

--*/

#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <memory>

// Forward declarations for Windows types
struct IDXGIOutput;
typedef void* HANDLE;

namespace vdd {

    /// <summary>
    /// Version information for the VDD SDK
    /// </summary>
    struct Version {
        int major;
        int minor;
        int patch;
    };

    /// <summary>
    /// Status codes returned by VDD SDK functions
    /// </summary>
    enum class Status {
        Ok = 0,                    // Operation succeeded
        AlreadyInstalled = 1,      // Driver already installed
        NotInstalled = 2,          // Driver not installed
        NotActive = 3,             // Driver not active
        Busy = 4,                  // Operation in progress
        AdminRequired = 5,         // Administrator privileges required
        DriverError = 6,           // Driver operation failed
        Timeout = 7,               // Operation timed out
        InvalidArg = 8,            // Invalid argument provided
        OsUnsupported = 9,         // Operating system not supported
        NotFound = 10,            // Resource not found
        AccessDenied = 11,         // Access denied
        OutOfMemory = 12,         // Insufficient memory
        InvalidState = 13,        // Invalid operation for current state
        ServiceUnavailable = 14,   // VDD service not available
        RebootRequired = 15,       // System reboot required to complete operation
        LeaseExpired = 16,        // Session lease expired
        ConcurrentAccess = 17,     // Concurrent access not allowed
        OperationNotPermitted = 18 // Operation not permitted for safety reasons
    };

    /// <summary>
    /// Display mode information
    /// </summary>
    struct DisplayMode {
        uint32_t width;
        uint32_t height;
        uint32_t refreshNumerator;
        uint32_t refreshDenominator;
    };

    /// <summary>
    /// Display rectangle for positioning
    /// </summary>
    struct DisplayRect {
        int32_t x;
        int32_t y;
        uint32_t width;
        uint32_t height;
    };

    /// <summary>
    /// Virtual display description for activation
    /// </summary>
    struct VirtualDisplayDesc {
        std::string name;                    // Friendly name reported via EDID
        DisplayMode preferredMode;           // Initial display mode
        bool hdr10 = false;                 // HDR10 support flag
        bool stereoscopic = false;          // 3D stereo support flag
        std::string manufacturer = "VDD";   // Manufacturer name
        std::string model = "Virtual Display"; // Model name
        std::string serialNumber = "";      // Serial number (empty for auto-generated)
    };

    /// <summary>
    /// Adapter information for enumeration
    /// </summary>
    struct AdapterInfo {
        std::wstring deviceInstanceId;     // PnP device instance ID
        bool isVirtual;                     // true for VDD adapters
        bool isActive;                      // currently enumerated and enabled
        bool isPrimary;                     // primary display adapter
        std::string name;                   // friendly display name
        std::string manufacturer;           // manufacturer name
        std::string model;                  // model name
        std::string serialNumber;          // serial number
        uint32_t adapterId;                 // adapter index
        uint32_t outputCount;              // number of outputs
    };

    /// <summary>
    /// Session lease handle for fault-tolerant operations
    /// </summary>
    struct LeaseHandle {
        uint64_t id = 0;
        
        bool IsValid() const { return id != 0; }
        bool operator==(const LeaseHandle& other) const { return id == other.id; }
        bool operator!=(const LeaseHandle& other) const { return id != other.id; }
    };

    /// <summary>
    /// Options for activating virtual displays with session management
    /// </summary>
    struct ActivateOptions {
        VirtualDisplayDesc desc;            // Display description
        uint32_t count = 1;                 // Number of displays to create
        bool autoRestoreOnCrash = true;     // Auto-restore on service crash
        uint32_t heartbeatIntervalMs = 1500; // Heartbeat interval in milliseconds
        uint32_t leaseTimeoutMs = 30000;   // Lease timeout in milliseconds
    };

    /// <summary>
    /// Configuration options for the VDD SDK
    /// </summary>
    struct SdkConfig {
        std::wstring serviceName = L"VddService";
        std::wstring pipeName = L"\\\\.\\pipe\\VddService";
        uint32_t defaultTimeoutMs = 15000;
        bool enableLogging = false;
        std::wstring logFilePath = L"";
        uint32_t maxLogLevel = 2; // 0=Trace, 1=Debug, 2=Info, 3=Warn, 4=Error
    };

    // ============================================================================
    // Core API Functions
    // ============================================================================

    /// <summary>
    /// Initialize the VDD SDK with configuration
    /// </summary>
    /// <param name="config">SDK configuration options</param>
    /// <returns>Status of the initialization</returns>
    Status Initialize(const SdkConfig& config = SdkConfig{});

    /// <summary>
    /// Shutdown the VDD SDK and cleanup resources
    /// </summary>
    /// <returns>Status of the shutdown</returns>
    Status Shutdown();

    /// <summary>
    /// Get the version of the VDD SDK
    /// </summary>
    /// <returns>Version information</returns>
    Version GetVersion();

    // ============================================================================
    // Driver Lifecycle Management
    // ============================================================================

    /// <summary>
    /// Install the VDD driver package (requires administrator privileges)
    /// </summary>
    /// <param name="infPath">Path to the driver INF file</param>
    /// <returns>Status of the installation</returns>
    Status InstallDriver(const std::wstring& infPath);

    /// <summary>
    /// Uninstall the VDD driver package (requires administrator privileges)
    /// </summary>
    /// <returns>Status of the uninstallation</returns>
    Status UninstallDriver();

    /// <summary>
    /// Check if the VDD driver is installed
    /// </summary>
    /// <returns>true if installed, false otherwise</returns>
    bool IsDriverInstalled();

    /// <summary>
    /// Get the version of the installed driver
    /// </summary>
    /// <returns>Driver version, or empty if not installed</returns>
    Version GetDriverVersion();

    // ============================================================================
    // Runtime Control
    // ============================================================================

    /// <summary>
    /// Activate virtual displays (hot-plug)
    /// </summary>
    /// <param name="desc">Display description</param>
    /// <param name="count">Number of displays to create</param>
    /// <returns>Status of the activation</returns>
    Status Activate(const VirtualDisplayDesc& desc, uint32_t count = 1);

    /// <summary>
    /// Deactivate all virtual displays (hot-unplug)
    /// </summary>
    /// <returns>Status of the deactivation</returns>
    Status Deactivate();

    /// <summary>
    /// Check if any virtual displays are currently active
    /// </summary>
    /// <returns>true if active, false otherwise</returns>
    bool IsActive();

    /// <summary>
    /// Get the number of currently active virtual displays
    /// </summary>
    /// <returns>Number of active displays</returns>
    uint32_t GetActiveDisplayCount();

    // ============================================================================
    // Display Configuration
    // ============================================================================

    /// <summary>
    /// Change the display mode for a specific output
    /// </summary>
    /// <param name="outputIndex">Index of the output to configure</param>
    /// <param name="mode">New display mode</param>
    /// <returns>Status of the mode change</returns>
    Status SetMode(uint32_t outputIndex, const DisplayMode& mode);

    /// <summary>
    /// Set the position of a virtual display in desktop space
    /// </summary>
    /// <param name="outputIndex">Index of the output to position</param>
    /// <param name="rect">New position and size</param>
    /// <returns>Status of the position change</returns>
    Status SetLocation(uint32_t outputIndex, const DisplayRect& rect);

    /// <summary>
    /// Set a virtual display as the primary display
    /// WARNING: This operation is dangerous and may cause login issues!
    /// </summary>
    /// <param name="outputIndex">Index of the output to set as primary</param>
    /// <param name="force">Force flag - must be true to override safety checks. Default is false (operation will be rejected)</param>
    /// <returns>Status of the primary display change</returns>
    Status SetPrimary(uint32_t outputIndex, bool force = false);

    /// <summary>
    /// Get the current mode of a specific output
    /// </summary>
    /// <param name="outputIndex">Index of the output</param>
    /// <param name="mode">Current mode (output parameter)</param>
    /// <returns>Status of the query</returns>
    Status GetMode(uint32_t outputIndex, DisplayMode& mode);

    /// <summary>
    /// Get the current location of a specific output
    /// </summary>
    /// <param name="outputIndex">Index of the output</param>
    /// <param name="rect">Current location (output parameter)</param>
    /// <returns>Status of the query</returns>
    Status GetLocation(uint32_t outputIndex, DisplayRect& rect);

    // ============================================================================
    // Display Enumeration and Query
    // ============================================================================

    /// <summary>
    /// Enumerate all display adapters (physical and virtual)
    /// </summary>
    /// <param name="adapters">Output vector of adapter information</param>
    /// <returns>Status of the enumeration</returns>
    Status EnumerateAdapters(std::vector<AdapterInfo>& adapters);

    /// <summary>
    /// Get supported display modes for a specific output
    /// </summary>
    /// <param name="outputIndex">Index of the output</param>
    /// <param name="modes">Output vector of supported modes</param>
    /// <returns>Status of the query</returns>
    Status EnumerateModes(uint32_t outputIndex, std::vector<DisplayMode>& modes);

    /// <summary>
    /// Find a DXGI output by name for fullscreen targeting
    /// </summary>
    /// <param name="name">Display name to search for</param>
    /// <param name="ppOutput">Output pointer to IDXGIOutput interface</param>
    /// <returns>Status of the search</returns>
    Status FindDxgiOutputByName(const std::string& name, void** ppOutput);

    // ============================================================================
    // Fault-Tolerant Session Management
    // ============================================================================

    /// <summary>
    /// Begin a fault-tolerant session with lease management
    /// </summary>
    /// <param name="lease">Output lease handle</param>
    /// <returns>Status of the session creation</returns>
    Status BeginSession(LeaseHandle& lease);

    /// <summary>
    /// Activate virtual displays under a session lease
    /// </summary>
    /// <param name="options">Activation options</param>
    /// <param name="lease">Session lease handle</param>
    /// <returns>Status of the activation</returns>
    Status ActivateLeased(const ActivateOptions& options, LeaseHandle lease);

    /// <summary>
    /// Send heartbeat to maintain session lease
    /// </summary>
    /// <param name="lease">Session lease handle</param>
    /// <returns>Status of the heartbeat</returns>
    Status Heartbeat(LeaseHandle lease);

    /// <summary>
    /// End a session and release the lease
    /// </summary>
    /// <param name="lease">Session lease handle to release</param>
    /// <returns>Status of the session termination</returns>
    Status EndSession(LeaseHandle lease);

    /// <summary>
    /// Get current session state information
    /// </summary>
    /// <param name="lease">Session lease handle</param>
    /// <param name="isActive">Output: whether session is active</param>
    /// <param name="timeRemainingMs">Output: time remaining in milliseconds</param>
    /// <returns>Status of the query</returns>
    Status GetSessionState(LeaseHandle lease, bool& isActive, uint32_t& timeRemainingMs);

    // ============================================================================
    // Recovery and Maintenance
    // ============================================================================

    /// <summary>
    /// Clean up orphaned VDD state and restore original desktop topology
    /// </summary>
    /// <returns>Status of the recovery operation</returns>
    Status RecoverOrphanedState();

    /// <summary>
    /// Ensure the VDD driver service is running
    /// </summary>
    /// <returns>Status of the service check/restart</returns>
    Status EnsureDriverRunning();

    /// <summary>
    /// Get detailed error information for the last operation
    /// </summary>
    /// <returns>Error message string</returns>
    std::string GetLastError();

    /// <summary>
    /// Get system information relevant to VDD operations
    /// </summary>
    /// <returns>System information string</returns>
    std::string GetSystemInfo();

    // ============================================================================
    // Advanced Features
    // ============================================================================

    /// <summary>
    /// Enable or disable HDR support for virtual displays
    /// </summary>
    /// <param name="outputIndex">Index of the output</param>
    /// <param name="enable">Enable HDR support</param>
    /// <returns>Status of the HDR configuration</returns>
    Status SetHdrSupport(uint32_t outputIndex, bool enable);

    /// <summary>
    /// Enable or disable stereoscopic 3D support
    /// </summary>
    /// <param name="outputIndex">Index of the output</param>
    /// <param name="enable">Enable stereo support</param>
    /// <returns>Status of the stereo configuration</returns>
    Status SetStereoSupport(uint32_t outputIndex, bool enable);

    /// <summary>
    /// Set custom EDID data for a virtual display
    /// </summary>
    /// <param name="outputIndex">Index of the output</param>
    /// <param name="edidData">EDID data (128 bytes)</param>
    /// <returns>Status of the EDID configuration</returns>
    Status SetCustomEdid(uint32_t outputIndex, const std::vector<uint8_t>& edidData);

    /// <summary>
    /// Get current EDID data for a virtual display
    /// </summary>
    /// <param name="outputIndex">Index of the output</param>
    /// <param name="edidData">Output EDID data</param>
    /// <returns>Status of the query</returns>
    Status GetEdid(uint32_t outputIndex, std::vector<uint8_t>& edidData);

    // ============================================================================
    // Utility Functions
    // ============================================================================

    /// <summary>
    /// Convert Status enum to human-readable string
    /// </summary>
    /// <param name="status">Status code</param>
    /// <returns>String representation</returns>
    std::string StatusToString(Status status);

    // Note: IsRunningAsAdministrator() and RequestElevation() are internal helper functions
    // They are used internally by InstallDriver/UninstallDriver and not exposed to users

} // namespace vdd

// ============================================================================
// C-style API for language interoperability
// ============================================================================

extern "C" {

    /// <summary>
    /// C-style API for basic VDD operations
    /// </summary>
    
    // Initialization
    vdd::Status VddInitialize(const vdd::SdkConfig* config);
    vdd::Status VddShutdown();
    vdd::Version VddGetVersion();

    // Driver management
    vdd::Status VddInstallDriver(const wchar_t* infPath);
    vdd::Status VddUninstallDriver();
    bool VddIsDriverInstalled();

    // Display control
    vdd::Status VddActivate(const char* name, uint32_t width, uint32_t height, uint32_t refreshRate, uint32_t count);
    vdd::Status VddDeactivate();
    bool VddIsActive();

    // Configuration
    vdd::Status VddSetMode(uint32_t outputIndex, uint32_t width, uint32_t height, uint32_t refreshRate);
    vdd::Status VddSetLocation(uint32_t outputIndex, int32_t x, int32_t y, uint32_t width, uint32_t height);
    vdd::Status VddSetPrimary(uint32_t outputIndex);

    // Query
    vdd::Status VddEnumerateAdapters(vdd::AdapterInfo* adapters, uint32_t* count);
    vdd::Status VddEnumerateModes(uint32_t outputIndex, vdd::DisplayMode* modes, uint32_t* count);

    // Error handling
    const char* VddGetLastError();
    const char* VddStatusToString(vdd::Status status);

}

// ============================================================================
// Convenience Macros
// ============================================================================

#define VDD_CHECK_STATUS(status) \
    do { \
        if ((status) != vdd::Status::Ok) { \
            return (status); \
        } \
    } while(0)

#define VDD_SAFE_CALL(func, ...) \
    do { \
        vdd::Status _status = (func)(__VA_ARGS__); \
        if (_status != vdd::Status::Ok) { \
            return _status; \
        } \
    } while(0)

// ============================================================================
// Version Information
// ============================================================================

#define VDD_SDK_VERSION_MAJOR 1
#define VDD_SDK_VERSION_MINOR 0
#define VDD_SDK_VERSION_PATCH 0
#define VDD_SDK_VERSION_STRING "1.0.0"

// ============================================================================
// Export Definitions
// ============================================================================

#ifdef VDD_SDK_EXPORTS
#define VDD_SDK_API __declspec(dllexport)
#else
#define VDD_SDK_API __declspec(dllimport)
#endif

// Apply export attribute to all public functions
#define VDD_API VDD_SDK_API
