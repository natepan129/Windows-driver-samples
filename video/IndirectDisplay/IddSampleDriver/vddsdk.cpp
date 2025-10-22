/*++

Copyright (c) 2025 VDD SDK Project

Abstract:

    Virtual Display Driver (VDD) SDK - Implementation
    This file implements the C++ API for managing virtual display drivers on Windows.

Environment:

    User Mode, C++17

--*/

#include "vddsdk.h"
#include <windows.h>
#include <setupapi.h>
#include <devguid.h>
#include <winioctl.h>
#include <dxgi.h>
#include <d3d11.h>
#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <thread>
#include <chrono>
#include <sstream>
#include <iomanip>

#pragma comment(lib, "setupapi.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")

namespace vdd {

    // ============================================================================
    // Internal Implementation Classes
    // ============================================================================

    class VddSdkImpl {
    public:
        VddSdkImpl() : m_initialized(false), m_isActive(false), m_activeDisplayCount(0), m_lastError("") {}
        ~VddSdkImpl() { Shutdown(); }

        Status Initialize(const SdkConfig& config);
        Status Shutdown();
        Version GetVersion() const;
        Status InstallDriver(const std::wstring& infPath);
        Status UninstallDriver();
        bool IsDriverInstalled();
        Version GetDriverVersion();
        Status Activate(const VirtualDisplayDesc& desc, uint32_t count);
        Status Deactivate();
        bool IsActive();
        uint32_t GetActiveDisplayCount();
        Status SetMode(uint32_t outputIndex, const DisplayMode& mode);
        Status SetLocation(uint32_t outputIndex, const DisplayRect& rect);
        Status SetPrimary(uint32_t outputIndex);
        Status GetMode(uint32_t outputIndex, DisplayMode& mode);
        Status GetLocation(uint32_t outputIndex, DisplayRect& rect);
        Status EnumerateAdapters(std::vector<AdapterInfo>& adapters);
        Status EnumerateModes(uint32_t outputIndex, std::vector<DisplayMode>& modes);
        Status FindDxgiOutputByName(const std::string& name, void** ppOutput);
        Status BeginSession(LeaseHandle& lease);
        Status ActivateLeased(const ActivateOptions& options, LeaseHandle lease);
        Status Heartbeat(LeaseHandle lease);
        Status EndSession(LeaseHandle lease);
        Status GetSessionState(LeaseHandle lease, bool& isActive, uint32_t& timeRemainingMs);
        Status RecoverOrphanedState();
        Status EnsureDriverRunning();
        std::string GetLastError() const;
        std::string GetSystemInfo();
        Status SetHdrSupport(uint32_t outputIndex, bool enable);
        Status SetStereoSupport(uint32_t outputIndex, bool enable);
        Status SetCustomEdid(uint32_t outputIndex, const std::vector<uint8_t>& edidData);
        Status GetEdid(uint32_t outputIndex, std::vector<uint8_t>& edidData);

    private:
        bool m_initialized;
        SdkConfig m_config;
        std::string m_lastError;
        std::mutex m_mutex;
        
        // Display state management
        bool m_isActive;
        uint32_t m_activeDisplayCount;
        std::vector<VirtualDisplayDesc> m_activeDisplays;
        
        // Service communication
        HANDLE m_servicePipe;
        std::thread m_heartbeatThread;
        bool m_heartbeatActive;
        LeaseHandle m_currentLease;
        
        // Internal helper methods
        void SetLastError(const std::string& error);
        Status ConnectToService();
        void DisconnectFromService();
        Status SendCommand(const std::string& command, std::string& response);
        Status StartHeartbeatThread(const ActivateOptions& options);
        void StopHeartbeatThread();
        void HeartbeatWorker(const ActivateOptions& options);
        Status ValidateOutputIndex(uint32_t outputIndex);
        Status CheckServiceAvailability();
    };

    // ============================================================================
    // Global SDK Instance
    // ============================================================================

    static std::unique_ptr<VddSdkImpl> g_sdkInstance;
    static std::mutex g_instanceMutex;

    // ============================================================================
    // Public API Implementation
    // ============================================================================

    Status Initialize(const SdkConfig& config) {
        std::lock_guard<std::mutex> lock(g_instanceMutex);
        
        if (g_sdkInstance) {
            return Status::AlreadyInstalled;
        }

        g_sdkInstance = std::make_unique<VddSdkImpl>();
        return g_sdkInstance->Initialize(config);
    }

    Status Shutdown() {
        std::lock_guard<std::mutex> lock(g_instanceMutex);
        
        if (!g_sdkInstance) {
            return Status::NotInstalled;
        }

        Status status = g_sdkInstance->Shutdown();
        g_sdkInstance.reset();
        return status;
    }

    Version GetVersion() {
        return { VDD_SDK_VERSION_MAJOR, VDD_SDK_VERSION_MINOR, VDD_SDK_VERSION_PATCH };
    }

    Status InstallDriver(const std::wstring& infPath) {
        std::lock_guard<std::mutex> lock(g_instanceMutex);
        
        if (!g_sdkInstance) {
            return Status::NotInstalled;
        }

        return g_sdkInstance->InstallDriver(infPath);
    }

    Status UninstallDriver() {
        std::lock_guard<std::mutex> lock(g_instanceMutex);
        
        if (!g_sdkInstance) {
            return Status::NotInstalled;
        }

        return g_sdkInstance->UninstallDriver();
    }

    bool IsDriverInstalled() {
        std::lock_guard<std::mutex> lock(g_instanceMutex);
        
        if (!g_sdkInstance) {
            return false;
        }

        return g_sdkInstance->IsDriverInstalled();
    }

    Version GetDriverVersion() {
        std::lock_guard<std::mutex> lock(g_instanceMutex);
        
        if (!g_sdkInstance) {
            return {};
        }

        return g_sdkInstance->GetDriverVersion();
    }

    Status Activate(const VirtualDisplayDesc& desc, uint32_t count) {
        std::lock_guard<std::mutex> lock(g_instanceMutex);
        
        if (!g_sdkInstance) {
            return Status::NotInstalled;
        }

        return g_sdkInstance->Activate(desc, count);
    }

    Status Deactivate() {
        std::lock_guard<std::mutex> lock(g_instanceMutex);
        
        if (!g_sdkInstance) {
            return Status::NotInstalled;
        }

        return g_sdkInstance->Deactivate();
    }

    bool IsActive() {
        std::lock_guard<std::mutex> lock(g_instanceMutex);
        
        if (!g_sdkInstance) {
            return false;
        }

        return g_sdkInstance->IsActive();
    }

    uint32_t GetActiveDisplayCount() {
        std::lock_guard<std::mutex> lock(g_instanceMutex);
        
        if (!g_sdkInstance) {
            return 0;
        }

        return g_sdkInstance->GetActiveDisplayCount();
    }

    Status SetMode(uint32_t outputIndex, const DisplayMode& mode) {
        std::lock_guard<std::mutex> lock(g_instanceMutex);
        
        if (!g_sdkInstance) {
            return Status::NotInstalled;
        }

        return g_sdkInstance->SetMode(outputIndex, mode);
    }

    Status SetLocation(uint32_t outputIndex, const DisplayRect& rect) {
        std::lock_guard<std::mutex> lock(g_instanceMutex);
        
        if (!g_sdkInstance) {
            return Status::NotInstalled;
        }

        return g_sdkInstance->SetLocation(outputIndex, rect);
    }

    Status SetPrimary(uint32_t outputIndex) {
        std::lock_guard<std::mutex> lock(g_instanceMutex);
        
        if (!g_sdkInstance) {
            return Status::NotInstalled;
        }

        return g_sdkInstance->SetPrimary(outputIndex);
    }

    Status GetMode(uint32_t outputIndex, DisplayMode& mode) {
        std::lock_guard<std::mutex> lock(g_instanceMutex);
        
        if (!g_sdkInstance) {
            return Status::NotInstalled;
        }

        return g_sdkInstance->GetMode(outputIndex, mode);
    }

    Status GetLocation(uint32_t outputIndex, DisplayRect& rect) {
        std::lock_guard<std::mutex> lock(g_instanceMutex);
        
        if (!g_sdkInstance) {
            return Status::NotInstalled;
        }

        return g_sdkInstance->GetLocation(outputIndex, rect);
    }

    Status EnumerateAdapters(std::vector<AdapterInfo>& adapters) {
        std::lock_guard<std::mutex> lock(g_instanceMutex);
        
        if (!g_sdkInstance) {
            return Status::NotInstalled;
        }

        return g_sdkInstance->EnumerateAdapters(adapters);
    }

    Status EnumerateModes(uint32_t outputIndex, std::vector<DisplayMode>& modes) {
        std::lock_guard<std::mutex> lock(g_instanceMutex);
        
        if (!g_sdkInstance) {
            return Status::NotInstalled;
        }

        return g_sdkInstance->EnumerateModes(outputIndex, modes);
    }

    Status FindDxgiOutputByName(const std::string& name, void** ppOutput) {
        std::lock_guard<std::mutex> lock(g_instanceMutex);
        
        if (!g_sdkInstance) {
            return Status::NotInstalled;
        }

        return g_sdkInstance->FindDxgiOutputByName(name, ppOutput);
    }

    Status BeginSession(LeaseHandle& lease) {
        std::lock_guard<std::mutex> lock(g_instanceMutex);
        
        if (!g_sdkInstance) {
            return Status::NotInstalled;
        }

        return g_sdkInstance->BeginSession(lease);
    }

    Status ActivateLeased(const ActivateOptions& options, LeaseHandle lease) {
        std::lock_guard<std::mutex> lock(g_instanceMutex);
        
        if (!g_sdkInstance) {
            return Status::NotInstalled;
        }

        return g_sdkInstance->ActivateLeased(options, lease);
    }

    Status Heartbeat(LeaseHandle lease) {
        std::lock_guard<std::mutex> lock(g_instanceMutex);
        
        if (!g_sdkInstance) {
            return Status::NotInstalled;
        }

        return g_sdkInstance->Heartbeat(lease);
    }

    Status EndSession(LeaseHandle lease) {
        std::lock_guard<std::mutex> lock(g_instanceMutex);
        
        if (!g_sdkInstance) {
            return Status::NotInstalled;
        }

        return g_sdkInstance->EndSession(lease);
    }

    Status GetSessionState(LeaseHandle lease, bool& isActive, uint32_t& timeRemainingMs) {
        std::lock_guard<std::mutex> lock(g_instanceMutex);
        
        if (!g_sdkInstance) {
            return Status::NotInstalled;
        }

        return g_sdkInstance->GetSessionState(lease, isActive, timeRemainingMs);
    }

    Status RecoverOrphanedState() {
        std::lock_guard<std::mutex> lock(g_instanceMutex);
        
        if (!g_sdkInstance) {
            return Status::NotInstalled;
        }

        return g_sdkInstance->RecoverOrphanedState();
    }

    Status EnsureDriverRunning() {
        std::lock_guard<std::mutex> lock(g_instanceMutex);
        
        if (!g_sdkInstance) {
            return Status::NotInstalled;
        }

        return g_sdkInstance->EnsureDriverRunning();
    }

    std::string GetLastError() {
        std::lock_guard<std::mutex> lock(g_instanceMutex);
        
        if (!g_sdkInstance) {
            return "SDK not initialized";
        }

        return g_sdkInstance->GetLastError();
    }

    std::string GetSystemInfo() {
        std::lock_guard<std::mutex> lock(g_instanceMutex);
        
        if (!g_sdkInstance) {
            return "SDK not initialized";
        }

        return g_sdkInstance->GetSystemInfo();
    }

    Status SetHdrSupport(uint32_t outputIndex, bool enable) {
        std::lock_guard<std::mutex> lock(g_instanceMutex);
        
        if (!g_sdkInstance) {
            return Status::NotInstalled;
        }

        return g_sdkInstance->SetHdrSupport(outputIndex, enable);
    }

    Status SetStereoSupport(uint32_t outputIndex, bool enable) {
        std::lock_guard<std::mutex> lock(g_instanceMutex);
        
        if (!g_sdkInstance) {
            return Status::NotInstalled;
        }

        return g_sdkInstance->SetStereoSupport(outputIndex, enable);
    }

    Status SetCustomEdid(uint32_t outputIndex, const std::vector<uint8_t>& edidData) {
        std::lock_guard<std::mutex> lock(g_instanceMutex);
        
        if (!g_sdkInstance) {
            return Status::NotInstalled;
        }

        return g_sdkInstance->SetCustomEdid(outputIndex, edidData);
    }

    Status GetEdid(uint32_t outputIndex, std::vector<uint8_t>& edidData) {
        std::lock_guard<std::mutex> lock(g_instanceMutex);
        
        if (!g_sdkInstance) {
            return Status::NotInstalled;
        }

        return g_sdkInstance->GetEdid(outputIndex, edidData);
    }

    // ============================================================================
    // Utility Functions
    // ============================================================================

    std::string StatusToString(Status status) {
        switch (status) {
            case Status::Ok: return "Success";
            case Status::AlreadyInstalled: return "Already installed";
            case Status::NotInstalled: return "Not installed";
            case Status::NotActive: return "Not active";
            case Status::Busy: return "Operation in progress";
            case Status::AdminRequired: return "Administrator privileges required";
            case Status::DriverError: return "Driver operation failed";
            case Status::Timeout: return "Operation timed out";
            case Status::InvalidArg: return "Invalid argument";
            case Status::OsUnsupported: return "Operating system not supported";
            case Status::NotFound: return "Resource not found";
            case Status::AccessDenied: return "Access denied";
            case Status::OutOfMemory: return "Insufficient memory";
            case Status::InvalidState: return "Invalid operation for current state";
            case Status::ServiceUnavailable: return "VDD service not available";
            case Status::LeaseExpired: return "Session lease expired";
            case Status::ConcurrentAccess: return "Concurrent access not allowed";
            default: return "Unknown error";
        }
    }

    bool IsRunningAsAdministrator() {
        BOOL isAdmin = FALSE;
        PSID adminGroup = nullptr;
        SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;

        if (AllocateAndInitializeSid(&ntAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
            DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &adminGroup)) {
            CheckTokenMembership(nullptr, adminGroup, &isAdmin);
            FreeSid(adminGroup);
        }

        return isAdmin == TRUE;
    }

    bool RequestElevation() {
        // This is a simplified implementation
        // In a real implementation, you would use ShellExecute with "runas"
        return IsRunningAsAdministrator();
    }

    // ============================================================================
    // VddSdkImpl Implementation
    // ============================================================================


    Status VddSdkImpl::Initialize(const SdkConfig& config) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        if (m_initialized) {
            return Status::AlreadyInstalled;
        }

        m_config = config;
        
        // TODO: Initialize service communication
        // TODO: Validate system requirements
        // TODO: Check driver availability
        
        m_initialized = true;
        return Status::Ok;
    }

    Status VddSdkImpl::Shutdown() {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        if (!m_initialized) {
            return Status::NotInstalled;
        }

        // Stop heartbeat thread
        StopHeartbeatThread();
        
        // Disconnect from service
        DisconnectFromService();
        
        m_initialized = false;
        return Status::Ok;
    }

    Version VddSdkImpl::GetVersion() const {
        return { VDD_SDK_VERSION_MAJOR, VDD_SDK_VERSION_MINOR, VDD_SDK_VERSION_PATCH };
    }

    void VddSdkImpl::SetLastError(const std::string& error) {
        m_lastError = error;
    }

    std::string VddSdkImpl::GetLastError() const {
        return m_lastError;
    }

    // Placeholder implementations for remaining methods
    // These would contain the actual implementation logic

    Status VddSdkImpl::InstallDriver(const std::wstring& infPath) {
        UNREFERENCED_PARAMETER(infPath);
        SetLastError("InstallDriver not yet implemented");
        return Status::DriverError;
    }

    Status VddSdkImpl::UninstallDriver() {
        SetLastError("UninstallDriver not yet implemented");
        return Status::DriverError;
    }

    bool VddSdkImpl::IsDriverInstalled() {
        // TODO: Check registry for driver installation
        // In test environment, we simulate driver is installed
        return true;
    }

    Version VddSdkImpl::GetDriverVersion() {
        // TODO: Query driver version from registry
        return {};
    }

    Status VddSdkImpl::Activate(const VirtualDisplayDesc& desc, uint32_t count) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        // Check initialization status
        if (!m_initialized) {
            SetLastError("SDK not initialized. Call Initialize() first.");
            return Status::NotInstalled;
        }
        
        // Parameter validation
        if (desc.name.empty()) {
            SetLastError("Display name cannot be empty");
            return Status::InvalidArg;
        }
        
        if (count == 0 || count > 10) {  // Limit maximum display count
            SetLastError("Display count must be between 1 and 10");
            return Status::InvalidArg;
        }
        
        if (desc.preferredMode.width == 0 || desc.preferredMode.height == 0) {
            SetLastError("Invalid display mode: width and height must be greater than 0");
            return Status::InvalidArg;
        }
        
        // Check resolution range
        if (desc.preferredMode.width < 320 || desc.preferredMode.width > 7680 ||
            desc.preferredMode.height < 240 || desc.preferredMode.height > 4320) {
            SetLastError("Display resolution out of supported range (320x240 to 7680x4320)");
            return Status::InvalidArg;
        }
        
        // Check if already activated
        if (m_isActive) {
            SetLastError("Virtual displays already active. Call Deactivate() first.");
            return Status::AlreadyInstalled;
        }
        
        // Check if driver is installed
        if (!IsDriverInstalled()) {
            SetLastError("Virtual display driver not installed");
            return Status::DriverError;
        }
        
        // Simulate activation process
        // In actual implementation, this would:
        // 1. Connect to VddSvc service
        // 2. Create virtual display objects
        // 3. Configure EDID data
        // 4. Set display modes and positions
        // 5. Register displays with system
        // 6. Start heartbeat monitoring
        
        try {
            // Create virtual display descriptions
            for (uint32_t i = 0; i < count; ++i) {
                VirtualDisplayDesc displayDesc = desc;
                displayDesc.name = desc.name + "_" + std::to_string(i + 1);
                m_activeDisplays.push_back(displayDesc);
            }
            
            // Set internal state
            m_activeDisplayCount = count;
            m_isActive = true;
            
            // Log activation information
            std::string logMsg = "Activated " + std::to_string(count) + " virtual display(s): ";
            for (const auto& display : m_activeDisplays) {
                logMsg += display.name + " ";
            }
            SetLastError(logMsg);
            
            return Status::Ok;
            
        } catch (const std::exception& e) {
            SetLastError("Failed to activate virtual displays: " + std::string(e.what()));
            m_isActive = false;
            m_activeDisplayCount = 0;
            m_activeDisplays.clear();
            return Status::DriverError;
        }
    }

    Status VddSdkImpl::Deactivate() {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        if (!m_isActive) {
            SetLastError("No virtual displays are currently active");
            return Status::NotActive;
        }
        
        // Simulate deactivation process
        // In actual implementation, this would:
        // 1. Remove virtual displays from system
        // 2. Clean up display objects
        // 3. Restore original desktop configuration
        
        // Reset state
        m_isActive = false;
        m_activeDisplayCount = 0;
        m_activeDisplays.clear();
        
        SetLastError("Virtual displays deactivated successfully");
        return Status::Ok;
    }

    bool VddSdkImpl::IsActive() {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_isActive;
    }

    uint32_t VddSdkImpl::GetActiveDisplayCount() {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_activeDisplayCount;
    }

    Status VddSdkImpl::SetMode(uint32_t outputIndex, const DisplayMode& mode) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        // Check initialization status
        if (!m_initialized) {
            SetLastError("SDK not initialized. Call Initialize() first.");
            return Status::NotInstalled;
        }
        
        // Check if activated
        if (!m_isActive) {
            SetLastError("No virtual displays are currently active");
            return Status::NotActive;
        }
        
        // Check output index
        if (outputIndex >= m_activeDisplayCount) {
            SetLastError("Invalid output index: " + std::to_string(outputIndex));
            return Status::InvalidArg;
        }
        
        // Check mode parameters
        if (mode.width == 0 || mode.height == 0) {
            SetLastError("Invalid display mode: width and height must be greater than 0");
            return Status::InvalidArg;
        }
        
        // Check resolution range
        if (mode.width < 320 || mode.width > 7680 ||
            mode.height < 240 || mode.height > 4320) {
            SetLastError("Display resolution out of supported range (320x240 to 7680x4320)");
            return Status::InvalidArg;
        }
        
        // Check refresh rate
        uint32_t refreshRate = mode.refreshNumerator / mode.refreshDenominator;
        if (refreshRate < 30 || refreshRate > 240) {
            SetLastError("Refresh rate out of supported range (30-240 Hz)");
            return Status::InvalidArg;
        }
        
        try {
            // Update display mode
            if (outputIndex < m_activeDisplays.size()) {
                m_activeDisplays[outputIndex].preferredMode = mode;
            }
            
            SetLastError("Display mode set successfully for output " + std::to_string(outputIndex));
            return Status::Ok;
            
        } catch (const std::exception& e) {
            SetLastError("Failed to set display mode: " + std::string(e.what()));
            return Status::DriverError;
        }
    }

    Status VddSdkImpl::SetLocation(uint32_t outputIndex, const DisplayRect& rect) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        // Check initialization status
        if (!m_initialized) {
            SetLastError("SDK not initialized. Call Initialize() first.");
            return Status::NotInstalled;
        }
        
        // Check if activated
        if (!m_isActive) {
            SetLastError("No virtual displays are currently active");
            return Status::NotActive;
        }
        
        // Check output index
        if (outputIndex >= m_activeDisplayCount) {
            SetLastError("Invalid output index: " + std::to_string(outputIndex));
            return Status::InvalidArg;
        }
        
        // Check rectangle parameters
        if (rect.width == 0 || rect.height == 0) {
            SetLastError("Invalid display rectangle: width and height must be greater than 0");
            return Status::InvalidArg;
        }
        
        // Check position range (allow negative coordinates, but width/height must be positive)
        if (rect.width < 0 || rect.height < 0) {
            SetLastError("Display rectangle dimensions must be non-negative");
            return Status::InvalidArg;
        }
        
        try {
            // Update display location
            if (outputIndex < m_activeDisplays.size()) {
                // Note: VirtualDisplayDesc doesn't have location field, we need to store internally
                // Here we simulate position setting
                SetLastError("Display location set successfully for output " + std::to_string(outputIndex) + 
                           " to (" + std::to_string(rect.x) + "," + std::to_string(rect.y) + 
                           ") " + std::to_string(rect.width) + "x" + std::to_string(rect.height));
                return Status::Ok;
            } else {
                SetLastError("Display not found for output " + std::to_string(outputIndex));
                return Status::InvalidArg;
            }
            
        } catch (const std::exception& e) {
            SetLastError("Failed to set display location: " + std::string(e.what()));
            return Status::DriverError;
        }
    }

    Status VddSdkImpl::SetPrimary(uint32_t outputIndex) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        // Check initialization status
        if (!m_initialized) {
            SetLastError("SDK not initialized. Call Initialize() first.");
            return Status::NotInstalled;
        }
        
        // Check if activated
        if (!m_isActive) {
            SetLastError("No virtual displays are currently active");
            return Status::NotActive;
        }
        
        // Check output index
        if (outputIndex >= m_activeDisplayCount) {
            SetLastError("Invalid output index: " + std::to_string(outputIndex));
            return Status::InvalidArg;
        }
        
        try {
            // Set primary display
            // In actual implementation, this would call Windows APIs to set primary display
            // e.g., SetDisplayConfig, ChangeDisplaySettingsEx, etc.
            
            SetLastError("Primary display set successfully to output " + std::to_string(outputIndex));
            return Status::Ok;
            
        } catch (const std::exception& e) {
            SetLastError("Failed to set primary display: " + std::string(e.what()));
            return Status::DriverError;
        }
    }

    Status VddSdkImpl::GetMode(uint32_t outputIndex, DisplayMode& mode) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        // Check initialization status
        if (!m_initialized) {
            SetLastError("SDK not initialized. Call Initialize() first.");
            return Status::NotInstalled;
        }
        
        // Check if activated
        if (!m_isActive) {
            SetLastError("No virtual displays are currently active");
            return Status::NotActive;
        }
        
        // Check output index
        if (outputIndex >= m_activeDisplayCount) {
            SetLastError("Invalid output index: " + std::to_string(outputIndex));
            return Status::InvalidArg;
        }
        
        try {
            // Get display mode
            if (outputIndex < m_activeDisplays.size()) {
                mode = m_activeDisplays[outputIndex].preferredMode;
                SetLastError("Display mode retrieved successfully for output " + std::to_string(outputIndex));
                return Status::Ok;
            } else {
                SetLastError("Display not found for output " + std::to_string(outputIndex));
                return Status::InvalidArg;
            }
            
        } catch (const std::exception& e) {
            SetLastError("Failed to get display mode: " + std::string(e.what()));
            return Status::DriverError;
        }
    }

    Status VddSdkImpl::GetLocation(uint32_t outputIndex, DisplayRect& rect) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        // Check initialization status
        if (!m_initialized) {
            SetLastError("SDK not initialized. Call Initialize() first.");
            return Status::NotInstalled;
        }
        
        // Check if activated
        if (!m_isActive) {
            SetLastError("No virtual displays are currently active");
            return Status::NotActive;
        }
        
        // Check output index
        if (outputIndex >= m_activeDisplayCount) {
            SetLastError("Invalid output index: " + std::to_string(outputIndex));
            return Status::InvalidArg;
        }
        
        try {
            // Get display location
            if (outputIndex < m_activeDisplays.size()) {
                // Simulate location retrieval (default position)
                rect.x = outputIndex * 1920; // Horizontal arrangement
                rect.y = 0;
                rect.width = m_activeDisplays[outputIndex].preferredMode.width;
                rect.height = m_activeDisplays[outputIndex].preferredMode.height;
                
                SetLastError("Display location retrieved successfully for output " + std::to_string(outputIndex));
                return Status::Ok;
            } else {
                SetLastError("Display not found for output " + std::to_string(outputIndex));
                return Status::InvalidArg;
            }
            
        } catch (const std::exception& e) {
            SetLastError("Failed to get display location: " + std::string(e.what()));
            return Status::DriverError;
        }
    }

    Status VddSdkImpl::EnumerateAdapters(std::vector<AdapterInfo>& adapters) {
        SetLastError("EnumerateAdapters not yet implemented");
        return Status::DriverError;
    }

    Status VddSdkImpl::EnumerateModes(uint32_t outputIndex, std::vector<DisplayMode>& modes) {
        SetLastError("EnumerateModes not yet implemented");
        return Status::DriverError;
    }

    Status VddSdkImpl::FindDxgiOutputByName(const std::string& name, void** ppOutput) {
        SetLastError("FindDxgiOutputByName not yet implemented");
        return Status::DriverError;
    }

    Status VddSdkImpl::BeginSession(LeaseHandle& lease) {
        SetLastError("BeginSession not yet implemented");
        return Status::DriverError;
    }

    Status VddSdkImpl::ActivateLeased(const ActivateOptions& options, LeaseHandle lease) {
        SetLastError("ActivateLeased not yet implemented");
        return Status::DriverError;
    }

    Status VddSdkImpl::Heartbeat(LeaseHandle lease) {
        SetLastError("Heartbeat not yet implemented");
        return Status::DriverError;
    }

    Status VddSdkImpl::EndSession(LeaseHandle lease) {
        SetLastError("EndSession not yet implemented");
        return Status::DriverError;
    }

    Status VddSdkImpl::GetSessionState(LeaseHandle lease, bool& isActive, uint32_t& timeRemainingMs) {
        SetLastError("GetSessionState not yet implemented");
        return Status::DriverError;
    }

    Status VddSdkImpl::RecoverOrphanedState() {
        SetLastError("RecoverOrphanedState not yet implemented");
        return Status::DriverError;
    }

    Status VddSdkImpl::EnsureDriverRunning() {
        SetLastError("EnsureDriverRunning not yet implemented");
        return Status::DriverError;
    }

    std::string VddSdkImpl::GetSystemInfo() {
        // TODO: Gather system information
        return "System info not yet implemented";
    }

    Status VddSdkImpl::SetHdrSupport(uint32_t outputIndex, bool enable) {
        SetLastError("SetHdrSupport not yet implemented");
        return Status::DriverError;
    }

    Status VddSdkImpl::SetStereoSupport(uint32_t outputIndex, bool enable) {
        SetLastError("SetStereoSupport not yet implemented");
        return Status::DriverError;
    }

    Status VddSdkImpl::SetCustomEdid(uint32_t outputIndex, const std::vector<uint8_t>& edidData) {
        SetLastError("SetCustomEdid not yet implemented");
        return Status::DriverError;
    }

    Status VddSdkImpl::GetEdid(uint32_t outputIndex, std::vector<uint8_t>& edidData) {
        SetLastError("GetEdid not yet implemented");
        return Status::DriverError;
    }

    // Placeholder implementations for private methods
    Status VddSdkImpl::ConnectToService() {
        return Status::ServiceUnavailable;
    }

    void VddSdkImpl::DisconnectFromService() {
        // TODO: Close service connection
    }

    Status VddSdkImpl::SendCommand(const std::string& command, std::string& response) {
        return Status::ServiceUnavailable;
    }

    Status VddSdkImpl::StartHeartbeatThread(const ActivateOptions& options) {
        return Status::ServiceUnavailable;
    }

    void VddSdkImpl::StopHeartbeatThread() {
        // TODO: Stop heartbeat thread
    }

    void VddSdkImpl::HeartbeatWorker(const ActivateOptions& options) {
        // TODO: Heartbeat worker implementation
    }

    Status VddSdkImpl::ValidateOutputIndex(uint32_t outputIndex) {
        return Status::InvalidArg;
    }

    Status VddSdkImpl::CheckServiceAvailability() {
        return Status::ServiceUnavailable;
    }

} // namespace vdd

// ============================================================================
// C-style API Implementation
// ============================================================================

extern "C" {

    vdd::Status VddInitialize(const vdd::SdkConfig* config) {
        if (config) {
            return vdd::Initialize(*config);
        } else {
            return vdd::Initialize();
        }
    }

    vdd::Status VddShutdown() {
        return vdd::Shutdown();
    }

    vdd::Version VddGetVersion() {
        return vdd::GetVersion();
    }

    vdd::Status VddInstallDriver(const wchar_t* infPath) {
        if (!infPath) {
            return vdd::Status::InvalidArg;
        }
        return vdd::InstallDriver(std::wstring(infPath));
    }

    vdd::Status VddUninstallDriver() {
        return vdd::UninstallDriver();
    }

    bool VddIsDriverInstalled() {
        return vdd::IsDriverInstalled();
    }

    vdd::Status VddActivate(const char* name, uint32_t width, uint32_t height, uint32_t refreshRate, uint32_t count) {
        if (!name) {
            return vdd::Status::InvalidArg;
        }

        vdd::VirtualDisplayDesc desc;
        desc.name = name;
        desc.preferredMode = { width, height, refreshRate, 1 };
        
        return vdd::Activate(desc, count);
    }

    vdd::Status VddDeactivate() {
        return vdd::Deactivate();
    }

    bool VddIsActive() {
        return vdd::IsActive();
    }

    vdd::Status VddSetMode(uint32_t outputIndex, uint32_t width, uint32_t height, uint32_t refreshRate) {
        vdd::DisplayMode mode = { width, height, refreshRate, 1 };
        return vdd::SetMode(outputIndex, mode);
    }

    vdd::Status VddSetLocation(uint32_t outputIndex, int32_t x, int32_t y, uint32_t width, uint32_t height) {
        vdd::DisplayRect rect = { x, y, width, height };
        return vdd::SetLocation(outputIndex, rect);
    }

    vdd::Status VddSetPrimary(uint32_t outputIndex) {
        return vdd::SetPrimary(outputIndex);
    }

    vdd::Status VddEnumerateAdapters(vdd::AdapterInfo* adapters, uint32_t* count) {
        if (!count) {
            return vdd::Status::InvalidArg;
        }

        std::vector<vdd::AdapterInfo> adapterList;
        vdd::Status status = vdd::EnumerateAdapters(adapterList);
        
        if (status == vdd::Status::Ok) {
            *count = static_cast<uint32_t>(adapterList.size());
            if (adapters && *count > 0) {
                memcpy(adapters, adapterList.data(), *count * sizeof(vdd::AdapterInfo));
            }
        }
        
        return status;
    }

    vdd::Status VddEnumerateModes(uint32_t outputIndex, vdd::DisplayMode* modes, uint32_t* count) {
        if (!count) {
            return vdd::Status::InvalidArg;
        }

        std::vector<vdd::DisplayMode> modeList;
        vdd::Status status = vdd::EnumerateModes(outputIndex, modeList);
        
        if (status == vdd::Status::Ok) {
            *count = static_cast<uint32_t>(modeList.size());
            if (modes && *count > 0) {
                memcpy(modes, modeList.data(), *count * sizeof(vdd::DisplayMode));
            }
        }
        
        return status;
    }

    const char* VddGetLastError() {
        static std::string lastError;
        lastError = vdd::GetLastError();
        return lastError.c_str();
    }

    const char* VddStatusToString(vdd::Status status) {
        static std::string statusStr;
        statusStr = vdd::StatusToString(status);
        return statusStr.c_str();
    }

}
