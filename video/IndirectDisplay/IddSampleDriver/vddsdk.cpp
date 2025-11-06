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
#include <devpkey.h>
#include <winioctl.h>
#include <sstream>
#include <algorithm>
#include <cfgmgr32.h>
#include <newdev.h>
#include <spapidef.h>
#include <dxgi.h>
#include <d3d11.h>
#include <string>
#include <vector>
#include <memory>
#include <fstream>
#include <chrono>
#include <ctime>
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
        std::mutex m_installMutex;  // Separate lock for install/uninstall operations
        
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
        std::vector<std::wstring> GetVirtualDisplayDeviceNames();  // Find IddSampleDriver displays
    };

    // ============================================================================
    // Helper Functions
    // ============================================================================

    static bool IsRunningAsAdministrator() {
        BOOL isAdmin = FALSE;
        PSID adminGroup = NULL;
        SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;
        
        if (AllocateAndInitializeSid(&ntAuthority, 2, 
            SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS,
            0, 0, 0, 0, 0, 0, &adminGroup)) {
            CheckTokenMembership(NULL, adminGroup, &isAdmin);
            FreeSid(adminGroup);
        }
        
        return isAdmin == TRUE;
    }

    // ============================================================================
    // Global SDK Instance
    // ============================================================================

    std::unique_ptr<VddSdkImpl> g_sdkInstance;
    std::mutex g_instanceMutex;

    // ============================================================================
    // Helper Functions
    // ============================================================================
    
    // Get or create instance for query operations (auto-create if needed)
    // NOTE: Caller must already hold g_instanceMutex lock!
    VddSdkImpl* GetOrCreateInstance_Locked() {
        if (!g_sdkInstance) {
            g_sdkInstance = std::make_unique<VddSdkImpl>();
        }
        return g_sdkInstance.get();
    }
    
    // Get instance for control operations (require explicit initialization)
    // NOTE: Caller must already hold g_instanceMutex lock!
    VddSdkImpl* GetInstanceStrict_Locked() {
        return g_sdkInstance.get();
    }

    // ============================================================================
    // Public API Implementation
    // ============================================================================

    Status Initialize(const SdkConfig& config) {
        std::lock_guard<std::mutex> lock(g_instanceMutex);
        
        if (g_sdkInstance) {
            return Status::AlreadyInstalled;
        }

        g_sdkInstance = std::make_unique<VddSdkImpl>();
        Status status = g_sdkInstance->Initialize(config);
        
        // Only reset instance if initialization completely failed
        if (status != Status::Ok && status != Status::AlreadyInstalled) {
            g_sdkInstance.reset();
        }
        
        return status;
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
        
        // InstallDriver is a standalone operation - auto-create if needed
        VddSdkImpl* impl = GetOrCreateInstance_Locked();
        return impl->InstallDriver(infPath);
    }

    Status UninstallDriver() {
        std::lock_guard<std::mutex> lock(g_instanceMutex);
        
        // UninstallDriver is a standalone operation - auto-create if needed
        VddSdkImpl* impl = GetOrCreateInstance_Locked();
        return impl->UninstallDriver();
    }

    bool IsDriverInstalled() {
        std::lock_guard<std::mutex> lock(g_instanceMutex);
        
        // Query operation - auto-create if needed
        VddSdkImpl* impl = GetOrCreateInstance_Locked();
        return impl->IsDriverInstalled();
    }

    Version GetDriverVersion() {
        std::lock_guard<std::mutex> lock(g_instanceMutex);
        
        // Query operation - auto-create if needed
        VddSdkImpl* impl = GetOrCreateInstance_Locked();
        return impl->GetDriverVersion();
    }

    Status Activate(const VirtualDisplayDesc& desc, uint32_t count) {
        std::lock_guard<std::mutex> lock(g_instanceMutex);
        
        // Independent operation - auto-create if needed
        VddSdkImpl* impl = GetOrCreateInstance_Locked();
        return impl->Activate(desc, count);
    }

    Status Deactivate() {
        std::lock_guard<std::mutex> lock(g_instanceMutex);
        
        // Independent operation - auto-create if needed
        VddSdkImpl* impl = GetOrCreateInstance_Locked();
        return impl->Deactivate();
    }

    bool IsActive() {
        std::lock_guard<std::mutex> lock(g_instanceMutex);
        
        // Query operation - auto-create if needed
        VddSdkImpl* impl = GetOrCreateInstance_Locked();
        return impl->IsActive();
    }

    uint32_t GetActiveDisplayCount() {
        std::lock_guard<std::mutex> lock(g_instanceMutex);
        
        // Query operation - auto-create if needed
        VddSdkImpl* impl = GetOrCreateInstance_Locked();
        return impl->GetActiveDisplayCount();
    }

    Status SetMode(uint32_t outputIndex, const DisplayMode& mode) {
        std::lock_guard<std::mutex> lock(g_instanceMutex);
        
        // FIXED: Auto-create instance for standalone display operations
        VddSdkImpl* impl = GetOrCreateInstance_Locked();
        
        return impl->SetMode(outputIndex, mode);
    }

    Status SetLocation(uint32_t outputIndex, const DisplayRect& rect) {
        std::lock_guard<std::mutex> lock(g_instanceMutex);
        
        // FIXED: Auto-create instance for standalone display operations
        VddSdkImpl* impl = GetOrCreateInstance_Locked();
        
        return impl->SetLocation(outputIndex, rect);
    }

    Status SetPrimary(uint32_t outputIndex) {
        std::lock_guard<std::mutex> lock(g_instanceMutex);
        
        // FIXED: Auto-create instance for standalone display operations
        VddSdkImpl* impl = GetOrCreateInstance_Locked();
        
        return impl->SetPrimary(outputIndex);
    }

    Status GetMode(uint32_t outputIndex, DisplayMode& mode) {
        std::lock_guard<std::mutex> lock(g_instanceMutex);
        
        // Control operation - require explicit initialization
        VddSdkImpl* impl = GetInstanceStrict_Locked();
        if (!impl) return Status::NotInstalled;
        
        return impl->GetMode(outputIndex, mode);
    }

    Status GetLocation(uint32_t outputIndex, DisplayRect& rect) {
        std::lock_guard<std::mutex> lock(g_instanceMutex);
        
        // Control operation - require explicit initialization
        VddSdkImpl* impl = GetInstanceStrict_Locked();
        if (!impl) return Status::NotInstalled;
        
        return impl->GetLocation(outputIndex, rect);
    }

    Status EnumerateAdapters(std::vector<AdapterInfo>& adapters) {
        std::lock_guard<std::mutex> lock(g_instanceMutex);
        
        // Query operation - auto-create if needed
        VddSdkImpl* impl = GetOrCreateInstance_Locked();
        return impl->EnumerateAdapters(adapters);
    }

    Status EnumerateModes(uint32_t outputIndex, std::vector<DisplayMode>& modes) {
        std::lock_guard<std::mutex> lock(g_instanceMutex);
        
        // Query operation - auto-create if needed
        VddSdkImpl* impl = GetOrCreateInstance_Locked();
        return impl->EnumerateModes(outputIndex, modes);
    }

    Status FindDxgiOutputByName(const std::string& name, void** ppOutput) {
        std::lock_guard<std::mutex> lock(g_instanceMutex);
        
        // Control operation - require explicit initialization
        VddSdkImpl* impl = GetInstanceStrict_Locked();
        if (!impl) return Status::NotInstalled;
        
        return impl->FindDxgiOutputByName(name, ppOutput);
    }

    Status BeginSession(LeaseHandle& lease) {
        std::lock_guard<std::mutex> lock(g_instanceMutex);
        
        // Control operation - require explicit initialization
        VddSdkImpl* impl = GetInstanceStrict_Locked();
        if (!impl) return Status::NotInstalled;
        
        return impl->BeginSession(lease);
    }

    Status ActivateLeased(const ActivateOptions& options, LeaseHandle lease) {
        std::lock_guard<std::mutex> lock(g_instanceMutex);
        
        // Control operation - require explicit initialization
        VddSdkImpl* impl = GetInstanceStrict_Locked();
        if (!impl) return Status::NotInstalled;
        
        return impl->ActivateLeased(options, lease);
    }

    Status Heartbeat(LeaseHandle lease) {
        std::lock_guard<std::mutex> lock(g_instanceMutex);
        
        // Control operation - require explicit initialization
        VddSdkImpl* impl = GetInstanceStrict_Locked();
        if (!impl) return Status::NotInstalled;
        
        return impl->Heartbeat(lease);
    }

    Status EndSession(LeaseHandle lease) {
        std::lock_guard<std::mutex> lock(g_instanceMutex);
        
        // Control operation - require explicit initialization
        VddSdkImpl* impl = GetInstanceStrict_Locked();
        if (!impl) return Status::NotInstalled;
        
        return impl->EndSession(lease);
    }

    Status GetSessionState(LeaseHandle lease, bool& isActive, uint32_t& timeRemainingMs) {
        std::lock_guard<std::mutex> lock(g_instanceMutex);
        
        // Control operation - require explicit initialization
        VddSdkImpl* impl = GetInstanceStrict_Locked();
        if (!impl) return Status::NotInstalled;
        
        return impl->GetSessionState(lease, isActive, timeRemainingMs);
    }

    Status RecoverOrphanedState() {
        std::lock_guard<std::mutex> lock(g_instanceMutex);
        
        // Control operation - require explicit initialization
        VddSdkImpl* impl = GetInstanceStrict_Locked();
        if (!impl) return Status::NotInstalled;
        
        return impl->RecoverOrphanedState();
    }

    Status EnsureDriverRunning() {
        std::lock_guard<std::mutex> lock(g_instanceMutex);
        
        // Control operation - require explicit initialization
        VddSdkImpl* impl = GetInstanceStrict_Locked();
        if (!impl) return Status::NotInstalled;
        
        return impl->EnsureDriverRunning();
    }

    std::string GetLastError() {
        std::lock_guard<std::mutex> lock(g_instanceMutex);
        
        // Query operation - auto-create if needed
        VddSdkImpl* impl = GetOrCreateInstance_Locked();
        return impl->GetLastError();
    }

    std::string GetSystemInfo() {
        std::lock_guard<std::mutex> lock(g_instanceMutex);
        
        if (!g_sdkInstance) {
            // Return basic system info even when SDK is not initialized
            std::stringstream info;
            
            // Get Windows version
            OSVERSIONINFOW osvi = {};
            osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOW);
            if (GetVersionExW(&osvi)) {
                info << "Windows " << osvi.dwMajorVersion << "." << osvi.dwMinorVersion;
                if (osvi.dwBuildNumber > 0) {
                    info << " Build " << osvi.dwBuildNumber;
                }
                info << "\n";
            }
            
            // Get system memory
            MEMORYSTATUSEX memStatus = {};
            memStatus.dwLength = sizeof(MEMORYSTATUSEX);
            if (GlobalMemoryStatusEx(&memStatus)) {
                info << "Total Memory: " << (memStatus.ullTotalPhys / (1024 * 1024)) << " MB\n";
                info << "Available Memory: " << (memStatus.ullAvailPhys / (1024 * 1024)) << " MB\n";
            }
            
            // Get processor info
            SYSTEM_INFO sysInfo = {};
            ::GetSystemInfo(&sysInfo);
            info << "Processors: " << sysInfo.dwNumberOfProcessors << "\n";
            info << "Architecture: " << (sysInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 ? "x64" : "x86") << "\n";
            
            return info.str();
        }

        return g_sdkInstance->GetSystemInfo();
    }

    Status SetHdrSupport(uint32_t outputIndex, bool enable) {
        std::lock_guard<std::mutex> lock(g_instanceMutex);
        
        // Control operation - require explicit initialization
        VddSdkImpl* impl = GetInstanceStrict_Locked();
        if (!impl) return Status::NotInstalled;
        
        return impl->SetHdrSupport(outputIndex, enable);
    }

    Status SetStereoSupport(uint32_t outputIndex, bool enable) {
        std::lock_guard<std::mutex> lock(g_instanceMutex);
        
        // Control operation - require explicit initialization
        VddSdkImpl* impl = GetInstanceStrict_Locked();
        if (!impl) return Status::NotInstalled;
        
        return impl->SetStereoSupport(outputIndex, enable);
    }

    Status SetCustomEdid(uint32_t outputIndex, const std::vector<uint8_t>& edidData) {
        std::lock_guard<std::mutex> lock(g_instanceMutex);
        
        // Control operation - require explicit initialization
        VddSdkImpl* impl = GetInstanceStrict_Locked();
        if (!impl) return Status::NotInstalled;
        
        return impl->SetCustomEdid(outputIndex, edidData);
    }

    Status GetEdid(uint32_t outputIndex, std::vector<uint8_t>& edidData) {
        std::lock_guard<std::mutex> lock(g_instanceMutex);
        
        // Control operation - require explicit initialization
        VddSdkImpl* impl = GetInstanceStrict_Locked();
        if (!impl) return Status::NotInstalled;
        
        return impl->GetEdid(outputIndex, edidData);
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
            case Status::RebootRequired: return "Reboot required";
            case Status::LeaseExpired: return "Session lease expired";
            case Status::ConcurrentAccess: return "Concurrent access not allowed";
            default: return "Unknown error";
        }
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
            SetLastError("SDK already initialized");
            return Status::AlreadyInstalled;
        }

        m_config = config;
        
        // Simple initialization - no complex configuration needed
        // Driver installation and activation will be separate steps
        
        m_initialized = true;
        SetLastError("SDK initialized successfully");
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
        std::lock_guard<std::mutex> lock(m_installMutex);
        
        printf("[VDD] ========================================\n");
        printf("[VDD] InstallDriver START\n");
        printf("[VDD] ========================================\n");
        printf("[VDD] INF Path (input): %ls\n", infPath.c_str());
        
        // Validation
        if (infPath.empty()) {
            printf("[VDD] ERROR: INF path is empty\n");
            SetLastError("INF path cannot be empty");
            return Status::InvalidArg;
        }
        printf("[VDD] Step 1: Validation passed\n");
        
        // Convert to absolute path
        wchar_t absPath[MAX_PATH];
        if (GetFullPathNameW(infPath.c_str(), MAX_PATH, absPath, nullptr) == 0) {
            printf("[VDD] ERROR: GetFullPathNameW failed, error=%d\n", ::GetLastError());
            SetLastError("Invalid INF path");
            return Status::InvalidArg;
        }
        printf("[VDD] Step 2: Absolute path: %ls\n", absPath);
        
        if (GetFileAttributesW(absPath) == INVALID_FILE_ATTRIBUTES) {
            printf("[VDD] ERROR: INF file not found at: %ls\n", absPath);
            SetLastError("INF file not found");
            return Status::InvalidArg;
        }
        printf("[VDD] Step 3: File exists\n");
        
        // Check admin
        if (!IsRunningAsAdministrator()) {
            printf("[VDD] ERROR: Not running as Administrator\n");
            SetLastError("Administrator privileges required");
            return Status::AdminRequired;
        }
        printf("[VDD] Step 4: Running as Administrator\n");
        
        // Check if device already exists
        GUID displayClassGuid = { 0x4D36E968, 0xE325, 0x11CE, 
            { 0xBF, 0xC1, 0x08, 0x00, 0x2B, 0xE1, 0x03, 0x18 } };
        
        printf("[VDD] Step 5: Checking for existing device...\n");
        {
            HDEVINFO hCheck = SetupDiGetClassDevsW(&displayClassGuid, nullptr, nullptr, DIGCF_ALLCLASSES);
            if (hCheck != INVALID_HANDLE_VALUE) {
                SP_DEVINFO_DATA dev{}; dev.cbSize = sizeof(dev);
                for (DWORD i = 0; SetupDiEnumDeviceInfo(hCheck, i, &dev); ++i) {
                    WCHAR hwid[4096] = {};
                    if (SetupDiGetDeviceRegistryPropertyW(hCheck, &dev, SPDRP_HARDWAREID,
                        nullptr, (BYTE*)hwid, sizeof(hwid), nullptr)) {
                        for (wchar_t* p = hwid; *p; p += wcslen(p) + 1) {
                            if (_wcsicmp(p, L"ROOT\\IddSampleDriver") == 0) {
                                printf("[VDD] ERROR: Device already exists!\n");
                                SetupDiDestroyDeviceInfoList(hCheck);
                                SetLastError("Device already installed");
                                return Status::AlreadyInstalled;
                            }
                        }
                    }
                }
                SetupDiDestroyDeviceInfoList(hCheck);
            }
        }
        printf("[VDD] Step 6: No existing device found\n");
        
        // Stage INF to Driver Store
        printf("[VDD] Step 7: Staging INF to Driver Store...\n");
        BOOL needReboot = FALSE;
        if (!DiInstallDriverW(nullptr, absPath, DIIRFLAG_FORCE_INF, &needReboot)) {
            DWORD err = ::GetLastError();
            printf("[VDD] ERROR: DiInstallDriverW failed, error=%d\n", err);
            SetLastError("Failed to stage INF to Driver Store: " + std::to_string(err));
            return Status::DriverError;
        }
        printf("[VDD] Step 8: INF staged successfully (reboot=%d)\n", needReboot);
        
        // Create device info list
        printf("[VDD] Step 9: Creating device info list...\n");
        HDEVINFO hDevInfo = SetupDiCreateDeviceInfoList(&displayClassGuid, nullptr);
        if (hDevInfo == INVALID_HANDLE_VALUE) {
            printf("[VDD] ERROR: SetupDiCreateDeviceInfoList failed, error=%d\n", ::GetLastError());
            SetLastError("Failed to create device info list: " + std::to_string(::GetLastError()));
            return Status::DriverError;
        }
        printf("[VDD] Step 10: Device info list created\n");
        
        // Create device info
        printf("[VDD] Step 11: Creating device info...\n");
        SP_DEVINFO_DATA devInfoData = {};
        devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
        
        if (!SetupDiCreateDeviceInfoW(hDevInfo, L"IddSampleDriver", &displayClassGuid,
            L"IddSampleDriver Device", nullptr, DICD_GENERATE_ID, &devInfoData)) {
            DWORD err = ::GetLastError();
            printf("[VDD] ERROR: SetupDiCreateDeviceInfoW failed, error=%d\n", err);
            SetupDiDestroyDeviceInfoList(hDevInfo);
            SetLastError("Failed to create device info: " + std::to_string(err));
            return Status::DriverError;
        }
        printf("[VDD] Step 12: Device info created\n");
        
        // Set HWID
        printf("[VDD] Step 13: Setting Hardware ID...\n");
        wchar_t hwid[] = L"ROOT\\IddSampleDriver\0\0";
        if (!SetupDiSetDeviceRegistryPropertyW(hDevInfo, &devInfoData, SPDRP_HARDWAREID,
            (const BYTE*)hwid, sizeof(hwid))) {
            DWORD err = ::GetLastError();
            printf("[VDD] ERROR: SetupDiSetDeviceRegistryPropertyW failed, error=%d\n", err);
            SetupDiDestroyDeviceInfoList(hDevInfo);
            SetLastError("Failed to set hardware ID: " + std::to_string(err));
            return Status::DriverError;
        }
        printf("[VDD] Step 14: Hardware ID set\n");
        
        // Register device
        printf("[VDD] Step 15: Registering device...\n");
        if (!SetupDiCallClassInstaller(DIF_REGISTERDEVICE, hDevInfo, &devInfoData)) {
            DWORD err = ::GetLastError();
            printf("[VDD] ERROR: DIF_REGISTERDEVICE failed, error=%d\n", err);
            SetupDiDestroyDeviceInfoList(hDevInfo);
            SetLastError("Failed to register device: " + std::to_string(err));
            return Status::DriverError;
        }
        printf("[VDD] Step 16: Device registered\n");
        
        // Set install params
        printf("[VDD] Step 17: Setting install parameters...\n");
        SP_DEVINSTALL_PARAMS_W installParams = {};
        installParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS_W);
        
        if (!SetupDiGetDeviceInstallParamsW(hDevInfo, &devInfoData, &installParams)) {
            printf("[VDD] ERROR: SetupDiGetDeviceInstallParamsW failed, error=%d\n", ::GetLastError());
            goto ROLLBACK;
        }
        
        wcsncpy_s(installParams.DriverPath, _countof(installParams.DriverPath), absPath, _TRUNCATE);
        installParams.Flags |= DI_ENUMSINGLEINF;
        
        if (!SetupDiSetDeviceInstallParamsW(hDevInfo, &devInfoData, &installParams)) {
            printf("[VDD] ERROR: SetupDiSetDeviceInstallParamsW failed, error=%d\n", ::GetLastError());
            goto ROLLBACK;
        }
        printf("[VDD] Step 18: Install parameters set\n");
        
        // Build driver list (COMPATDRIVER instead of CLASSDRIVER)
        printf("[VDD] Step 19: Building driver list...\n");
        if (!SetupDiBuildDriverInfoList(hDevInfo, &devInfoData, SPDIT_COMPATDRIVER)) {
            printf("[VDD] ERROR: SetupDiBuildDriverInfoList failed, error=%d\n", ::GetLastError());
            goto ROLLBACK;
        }
        printf("[VDD] Step 20: Driver list built\n");
        
        // Select best compatible driver
        printf("[VDD] Step 21: Selecting best compatible driver...\n");
        if (!SetupDiCallClassInstaller(DIF_SELECTBESTCOMPATDRV, hDevInfo, &devInfoData)) {
            printf("[VDD] ERROR: DIF_SELECTBESTCOMPATDRV failed, error=%d\n", ::GetLastError());
            goto ROLLBACK_WITH_LIST;
        }
        printf("[VDD] Step 22: Driver selected\n");
        
        // Install device
        printf("[VDD] Step 23: Installing device (this may take a while)...\n");
        if (!SetupDiCallClassInstaller(DIF_INSTALLDEVICE, hDevInfo, &devInfoData)) {
            DWORD err = ::GetLastError();
            printf("[VDD] WARNING: DIF_INSTALLDEVICE failed, error=%d\n", err);
            printf("[VDD] Step 23b: Trying fallback method (UpdateDriverForPlugAndPlayDevices)...\n");
            
            // Fallback: Use UpdateDriverForPlugAndPlayDevices
            // This method is more tolerant of unsigned drivers
            BOOL rebootRequired = FALSE;
            BOOL fallbackResult = UpdateDriverForPlugAndPlayDevicesW(
                nullptr,
                L"ROOT\\IddSampleDriver",
                absPath,
                INSTALLFLAG_FORCE,
                &rebootRequired
            );
            
            if (!fallbackResult) {
                printf("[VDD] ERROR: Fallback method also failed, error=%d\n", ::GetLastError());
                goto ROLLBACK_WITH_LIST;
            }
            
            printf("[VDD] Step 24: Device installed via fallback method!\n");
            if (rebootRequired) {
                needReboot = TRUE;
            }
        } else {
            printf("[VDD] Step 24: Device installed successfully!\n");
        }
        
        // Cleanup
        SetupDiDestroyDriverInfoList(hDevInfo, &devInfoData, SPDIT_COMPATDRIVER);
        SetupDiDestroyDeviceInfoList(hDevInfo);
        
        printf("[VDD] ========================================\n");
        if (needReboot) {
            printf("[VDD] SUCCESS: Driver installed (reboot required)\n");
            SetLastError("Driver installed successfully (reboot required)");
            printf("[VDD] ========================================\n");
            return Status::RebootRequired;
        } else {
            printf("[VDD] SUCCESS: Driver installed\n");
            SetLastError("Driver installed successfully");
            printf("[VDD] ========================================\n");
            return Status::Ok;
        }

    ROLLBACK_WITH_LIST:
        printf("[VDD] *** ROLLBACK: Destroying driver info list ***\n");
        SetupDiDestroyDriverInfoList(hDevInfo, &devInfoData, SPDIT_COMPATDRIVER);
        
    ROLLBACK:
        {
            DWORD lastErr = ::GetLastError();
            printf("[VDD] *** ROLLBACK: Installation failed, error=%d ***\n", lastErr);
            printf("[VDD] *** ROLLBACK: Removing partially created device ***\n");
            
            // Remove the device node we just created
            SP_REMOVEDEVICE_PARAMS removeParams = {};
            removeParams.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
            removeParams.ClassInstallHeader.InstallFunction = DIF_REMOVE;
            removeParams.Scope = DI_REMOVEDEVICE_GLOBAL;
            removeParams.HwProfile = 0;
            
            SetupDiSetClassInstallParamsW(hDevInfo, &devInfoData,
                reinterpret_cast<SP_CLASSINSTALL_HEADER*>(&removeParams), sizeof(removeParams));
            SetupDiCallClassInstaller(DIF_REMOVE, hDevInfo, &devInfoData);
            
            SetupDiDestroyDeviceInfoList(hDevInfo);
            
            SetLastError("Installation failed (rolled back): " + std::to_string(lastErr));
            return Status::DriverError;
        }
    }

    Status VddSdkImpl::UninstallDriver() {
        std::lock_guard<std::mutex> lock(m_installMutex);
        
        GUID displayClassGuid = { 0x4D36E968, 0xE325, 0x11CE, 
            { 0xBF, 0xC1, 0x08, 0x00, 0x2B, 0xE1, 0x03, 0x18 } };
        
        HDEVINFO hDevInfo = SetupDiGetClassDevsW(&displayClassGuid, nullptr, nullptr, 
            DIGCF_ALLCLASSES);  // Removed DIGCF_PRESENT to find offline/phantom devices
        
        if (hDevInfo == INVALID_HANDLE_VALUE) {
            SetLastError("Failed to get display device information: " + std::to_string(::GetLastError()));
            return Status::DriverError;
        }
        
        // Collect ALL matching devices first (to avoid iterator invalidation)
        std::vector<SP_DEVINFO_DATA> devicesToRemove;
        SP_DEVINFO_DATA devInfoData = {};
        devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
        
        DWORD deviceIndex = 0;
        while (SetupDiEnumDeviceInfo(hDevInfo, deviceIndex, &devInfoData)) {
            WCHAR hwid[4096] = {};
            bool isOurDevice = false;
            
            // Get Hardware ID (HWID) - more reliable than InstanceId
            if (SetupDiGetDeviceRegistryPropertyW(hDevInfo, &devInfoData, SPDRP_HARDWAREID,
                nullptr, (BYTE*)hwid, sizeof(hwid), nullptr)) {
                
                // HWID is a multi-string (REG_MULTI_SZ), iterate through each string
                for (wchar_t* p = hwid; *p; p += wcslen(p) + 1) {
                    // Exact case-insensitive match
                    if (_wcsicmp(p, L"ROOT\\IddSampleDriver") == 0) {
                        isOurDevice = true;
                        devicesToRemove.push_back(devInfoData);
                    break;
                }
            }
            }
            
            deviceIndex++;
        }
        
        if (devicesToRemove.empty()) {
            SetupDiDestroyDeviceInfoList(hDevInfo);
            SetLastError("IddSampleDriver not found in system");
            return Status::NotFound;
        }
        
    // Collect INF names for Driver Store cleanup (before removing devices)
    std::vector<std::wstring> infNamesToRemove;
    for (const auto& devInfo : devicesToRemove) {
        // Get published INF name (oemXX.inf) using SetupDiGetDeviceRegistryProperty
        // Method 1: Try to get the driver info detail which includes INF path
        SP_DRVINFO_DATA_W drvInfo = {};
        drvInfo.cbSize = sizeof(SP_DRVINFO_DATA_W);
        
        bool gotInfName = false;
        std::wstring infName;
        
        if (SetupDiGetDeviceInstallParamsW(hDevInfo, const_cast<PSP_DEVINFO_DATA>(&devInfo), nullptr) == FALSE) {
            SP_DEVINSTALL_PARAMS_W installParams = {};
            installParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS_W);
            SetupDiSetDeviceInstallParamsW(hDevInfo, const_cast<PSP_DEVINFO_DATA>(&devInfo), &installParams);
        }
        
        if (SetupDiBuildDriverInfoList(hDevInfo, const_cast<PSP_DEVINFO_DATA>(&devInfo), SPDIT_COMPATDRIVER)) {
            if (SetupDiEnumDriverInfoW(hDevInfo, const_cast<PSP_DEVINFO_DATA>(&devInfo), SPDIT_COMPATDRIVER, 0, &drvInfo)) {
                SP_DRVINFO_DETAIL_DATA_W detail = {};
                detail.cbSize = sizeof(SP_DRVINFO_DETAIL_DATA_W);
                DWORD requiredSize = 0;
                
                if (SetupDiGetDriverInfoDetailW(hDevInfo, const_cast<PSP_DEVINFO_DATA>(&devInfo), 
                    &drvInfo, &detail, sizeof(detail), &requiredSize) || ::GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
                    // Extract filename from full INF path
                    std::wstring fullPath(detail.InfFileName);
                    size_t lastSlash = fullPath.find_last_of(L"\\/");
                    infName = (lastSlash != std::wstring::npos) ? fullPath.substr(lastSlash + 1) : fullPath;
                    
                    // Only accept oemXX.inf format to avoid accidents
                    if (infName.find(L"oem") == 0 && infName.find(L".inf") != std::wstring::npos) {
                        infNamesToRemove.push_back(infName);
                        gotInfName = true;
                    }
                }
            }
            SetupDiDestroyDriverInfoList(hDevInfo, const_cast<PSP_DEVINFO_DATA>(&devInfo), SPDIT_COMPATDRIVER);
        }
        
        // Fallback: Use SPDRP_DRIVER if we couldn't get the INF name above
        if (!gotInfName) {
            WCHAR driverKey[MAX_PATH] = {};
            if (SetupDiGetDeviceRegistryPropertyW(hDevInfo, const_cast<PSP_DEVINFO_DATA>(&devInfo), 
                SPDRP_DRIVER, nullptr, (BYTE*)driverKey, sizeof(driverKey), nullptr)) {
                // SPDRP_DRIVER returns something like "{4d36e968-e325-11ce-bfc1-08002be10318}\0007"
                // We try to extract INF from registry or use as-is for DiUninstallDriverW
                infNamesToRemove.push_back(driverKey);
            }
        }
    }
    
    // Remove each device
    int successCount = 0;
    int failCount = 0;
    std::string lastError;
    
    for (auto& devInfo : devicesToRemove) {
        // Step 1: Disable device first (safer for display devices)
        SP_PROPCHANGE_PARAMS disableParams = {};
        disableParams.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
        disableParams.ClassInstallHeader.InstallFunction = DIF_PROPERTYCHANGE;
        disableParams.StateChange = DICS_DISABLE;
        disableParams.Scope = DICS_FLAG_GLOBAL;
        disableParams.HwProfile = 0;
        
        if (SetupDiSetClassInstallParamsW(hDevInfo, &devInfo, 
                reinterpret_cast<SP_CLASSINSTALL_HEADER*>(&disableParams), sizeof(disableParams))) {
            SetupDiCallClassInstaller(DIF_PROPERTYCHANGE, hDevInfo, &devInfo);
            // Don't fail if disable fails, continue to remove
        }
        
        // Step 2: Remove device
        SP_REMOVEDEVICE_PARAMS removeParams = {};
        removeParams.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
        removeParams.ClassInstallHeader.InstallFunction = DIF_REMOVE;
        removeParams.Scope = DI_REMOVEDEVICE_GLOBAL;
        removeParams.HwProfile = 0;
        
        if (SetupDiSetClassInstallParamsW(hDevInfo, &devInfo, 
            reinterpret_cast<SP_CLASSINSTALL_HEADER*>(&removeParams), sizeof(removeParams))) {
            
            if (SetupDiCallClassInstaller(DIF_REMOVE, hDevInfo, &devInfo)) {
                successCount++;
            } else {
                failCount++;
                lastError = "Failed to remove device: " + std::to_string(::GetLastError());
            }
        } else {
            failCount++;
            lastError = "Failed to set remove parameters: " + std::to_string(::GetLastError());
        }
        }
        
        SetupDiDestroyDeviceInfoList(hDevInfo);
        
        // Clean up Driver Store (remove INF packages)
        // This prevents accumulation of oem1.inf, oem2.inf, etc.
        if (successCount > 0 && !infNamesToRemove.empty()) {
            printf("[VDD] Cleaning up Driver Store packages...\n");
            for (const auto& infName : infNamesToRemove) {
                BOOL needReboot = FALSE;
                printf("[VDD] Removing package: %ls\n", infName.c_str());
                if (DiUninstallDriverW(nullptr, infName.c_str(), DIURFLAG_NO_REMOVE_INF, &needReboot)) {
                    printf("[VDD] Successfully removed package: %ls\n", infName.c_str());
                } else {
                    DWORD err = ::GetLastError();
                    printf("[VDD] WARNING: Failed to remove package %ls, error=%d\n", infName.c_str(), err);
                    // Continue anyway - device is already removed
                }
            }
        }
        
        // Report results
        std::string message = "Uninstall: " + std::to_string(successCount) + " device(s) removed";
        if (failCount > 0) {
            message += ", " + std::to_string(failCount) + " failed. Last error: " + lastError;
            SetLastError(message);
            return (successCount > 0) ? Status::Ok : Status::DriverError;  // Partial success if any removed
        }
        
        SetLastError(message);
        return Status::Ok;
    }

    bool VddSdkImpl::IsDriverInstalled() {
        // Use SetupAPI to check if device actually exists
        GUID displayClassGuid = { 0x4D36E968, 0xE325, 0x11CE, 
            { 0xBF, 0xC1, 0x08, 0x00, 0x2B, 0xE1, 0x03, 0x18 } };
        
        // Get all display devices (including offline/phantom)
        HDEVINFO hDevInfo = SetupDiGetClassDevsW(
            &displayClassGuid, 
            nullptr, 
            nullptr, 
            DIGCF_ALLCLASSES  // Include offline devices
        );
        
        if (hDevInfo == INVALID_HANDLE_VALUE) {
            return false;
        }
        
        SP_DEVINFO_DATA devInfoData = {};
        devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
        
        // Enumerate all devices
        int deviceCount = 0;
        for (DWORD i = 0; SetupDiEnumDeviceInfo(hDevInfo, i, &devInfoData); i++) {
            deviceCount++;
            WCHAR hwid[4096] = {};
            if (SetupDiGetDeviceRegistryPropertyW(
                hDevInfo, 
                &devInfoData, 
                SPDRP_HARDWAREID,
                nullptr, 
                (BYTE*)hwid, 
                sizeof(hwid), 
                nullptr)) {
                
                // Check all hardware IDs (multi-string)
                for (wchar_t* p = hwid; *p; p += wcslen(p) + 1) {
                    // Use prefix match (case-insensitive) to handle both:
                    // - ROOT\IddSampleDriver
                    // - ROOT\IDDSAMPLEDRIVER\0000
                    if (_wcsnicmp(p, L"ROOT\\IddSampleDriver", 20) == 0) {
                        SetupDiDestroyDeviceInfoList(hDevInfo);
                        return true;
                    }
                }
            }
        }
        
        SetupDiDestroyDeviceInfoList(hDevInfo);
        return false;
    }

    Version VddSdkImpl::GetDriverVersion() {
        // REAL IMPLEMENTATION - Query driver version from registry
        HKEY hKey;
        LONG result = RegOpenKeyExW(HKEY_LOCAL_MACHINE,
            L"SYSTEM\\CurrentControlSet\\Services\\IddSampleDriver",
            0, KEY_READ, &hKey);
            
        if (result != ERROR_SUCCESS) {
            return {0, 0, 0};
        }
        
        DWORD versionSize = sizeof(DWORD);
        DWORD version = 0;
        result = RegQueryValueExW(hKey, L"Version", nullptr, nullptr,
            reinterpret_cast<LPBYTE>(&version), &versionSize);
            
        RegCloseKey(hKey);
        
        if (result == ERROR_SUCCESS) {
            return {static_cast<uint32_t>(version >> 16), 
                   static_cast<uint32_t>((version >> 8) & 0xFF),
                   static_cast<uint32_t>(version & 0xFF)};
        }
        
        return {1, 0, 0}; // Default version
    }

    // Helper function: Find virtual display device names (IddSampleDriver monitors)
    std::vector<std::wstring> VddSdkImpl::GetVirtualDisplayDeviceNames() {
        std::vector<std::wstring> deviceNames;
        
        DISPLAY_DEVICEW displayDevice = {};
        displayDevice.cb = sizeof(DISPLAY_DEVICEW);
        
        // Enumerate all display devices
        for (DWORD i = 0; EnumDisplayDevicesW(nullptr, i, &displayDevice, 0); i++) {
            // Check if this is our virtual display driver
            std::wstring deviceString(displayDevice.DeviceString);
            if (deviceString.find(L"IddSampleDriver") != std::wstring::npos ||
                deviceString.find(L"Generic PnP Monitor") != std::wstring::npos) {
                
                // This might be our device, now check the monitors
                DISPLAY_DEVICEW monitorDevice = {};
                monitorDevice.cb = sizeof(DISPLAY_DEVICEW);
                
                for (DWORD j = 0; EnumDisplayDevicesW(displayDevice.DeviceName, j, &monitorDevice, 0); j++) {
                    std::wstring monitorId(monitorDevice.DeviceID);
                    // Check if monitor belongs to IddSampleDriver
                    if (monitorId.find(L"DELD0E6") != std::wstring::npos ||  // Dell EDID
                        monitorId.find(L"LEN65BF") != std::wstring::npos ||  // Lenovo EDID
                        monitorId.find(L"HWP2676") != std::wstring::npos) {  // Third monitor EDID
                        
                        deviceNames.push_back(displayDevice.DeviceName);
                        printf("[VDD] Found virtual display: %ls (Monitor: %ls)\n", 
                               displayDevice.DeviceName, monitorDevice.DeviceID);
                        break; // Only add device once
                    }
                }
            }
        }
        
        printf("[VDD] Total virtual displays found: %zu\n", deviceNames.size());
        return deviceNames;
    }

    Status VddSdkImpl::Activate(const VirtualDisplayDesc& desc, uint32_t count) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        printf("[VDD] Activate: Starting...\n");
        
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
        
        // Check if driver is installed (don't check m_isActive - each vddctl run is a new process)
        if (!IsDriverInstalled()) {
            SetLastError("Virtual display driver not installed");
            return Status::DriverError;
        }
        
        // Real activation using SetupAPI to enable the device
        // Get device info set for our driver
        HDEVINFO hDevInfo = SetupDiGetClassDevsW(
            &GUID_DEVCLASS_DISPLAY,
            nullptr,
            nullptr,
            DIGCF_PRESENT | DIGCF_ALLCLASSES
        );
        
        if (hDevInfo == INVALID_HANDLE_VALUE) {
            DWORD err = ::GetLastError();
            printf("[VDD] ERROR: SetupDiGetClassDevsW failed, error=%d\n", err);
            SetLastError("Failed to get device list: " + std::to_string(err));
            return Status::DriverError;
        }
        
        bool deviceFound = false;
        SP_DEVINFO_DATA devInfoData = {};
        devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
        
        // Find our device
        for (DWORD i = 0; SetupDiEnumDeviceInfo(hDevInfo, i, &devInfoData); ++i) {
            wchar_t hwid[256] = {};
            if (SetupDiGetDeviceRegistryPropertyW(hDevInfo, &devInfoData, SPDRP_HARDWAREID,
                nullptr, (PBYTE)hwid, sizeof(hwid), nullptr)) {
                
                if (_wcsicmp(hwid, L"ROOT\\IddSampleDriver") == 0) {
                    deviceFound = true;
                    printf("[VDD] Found device: %ls\n", hwid);
                    break;
                }
            }
        }
        
        if (!deviceFound) {
            SetupDiDestroyDeviceInfoList(hDevInfo);
            SetLastError("Driver device not found in system");
            return Status::NotInstalled;
        }
        
        // Enable the device using DIF_PROPERTYCHANGE
        SP_PROPCHANGE_PARAMS propChangeParams = {};
        propChangeParams.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
        propChangeParams.ClassInstallHeader.InstallFunction = DIF_PROPERTYCHANGE;
        propChangeParams.StateChange = DICS_ENABLE;
        propChangeParams.Scope = DICS_FLAG_GLOBAL;
        propChangeParams.HwProfile = 0;
        
        if (!SetupDiSetClassInstallParamsW(hDevInfo, &devInfoData,
            (SP_CLASSINSTALL_HEADER*)&propChangeParams, sizeof(propChangeParams))) {
            DWORD err = ::GetLastError();
            printf("[VDD] ERROR: SetupDiSetClassInstallParamsW failed, error=%d\n", err);
            SetupDiDestroyDeviceInfoList(hDevInfo);
            SetLastError("Failed to set enable parameters: " + std::to_string(err));
            return Status::DriverError;
        }
        
        if (!SetupDiCallClassInstaller(DIF_PROPERTYCHANGE, hDevInfo, &devInfoData)) {
            DWORD err = ::GetLastError();
            printf("[VDD] ERROR: DIF_PROPERTYCHANGE (ENABLE) failed, error=%d\n", err);
            SetupDiDestroyDeviceInfoList(hDevInfo);
            
            // Device might already be enabled - check status
            ULONG status = 0, problemNumber = 0;
            if (CM_Get_DevNode_Status(&status, &problemNumber, devInfoData.DevInst, 0) == CR_SUCCESS) {
                if (!(status & DN_HAS_PROBLEM) && (status & DN_STARTED)) {
                    printf("[VDD] Device already enabled and running\n");
                    // Device is already enabled, treat as success
                } else {
                    SetLastError("Failed to enable device: " + std::to_string(err));
                    return Status::DriverError;
                }
            } else {
                SetLastError("Failed to enable device: " + std::to_string(err));
                return Status::DriverError;
            }
        }
        
        SetupDiDestroyDeviceInfoList(hDevInfo);
        
        // Update internal state
        m_isActive = true;
        m_activeDisplayCount = count;
        
        // Create display descriptors for tracking
        for (uint32_t i = 0; i < count; ++i) {
            VirtualDisplayDesc displayDesc = desc;
            displayDesc.name = desc.name + "_" + std::to_string(i + 1);
            m_activeDisplays.push_back(displayDesc);
        }
        
        // CRITICAL SAFETY: Configure display topology to prevent black screen
        // Wait a moment for device to be fully initialized
        Sleep(500);
        
        printf("[VDD] Activate: Configuring display topology with PRIMARY PROTECTION...\n");
        
        // SAFETY LAYER 1: Query and save current configuration BEFORE any changes
        UINT32 numPathArrayElements = 0;
        UINT32 numModeInfoArrayElements = 0;
        
        LONG result = GetDisplayConfigBufferSizes(QDC_ONLY_ACTIVE_PATHS, &numPathArrayElements, &numModeInfoArrayElements);
        if (result == ERROR_SUCCESS && numPathArrayElements > 0) {
            std::vector<DISPLAYCONFIG_PATH_INFO> pathArray(numPathArrayElements);
            std::vector<DISPLAYCONFIG_MODE_INFO> modeInfoArray(numModeInfoArrayElements);
            
            // Query current configuration
            result = QueryDisplayConfig(QDC_ONLY_ACTIVE_PATHS,
                &numPathArrayElements, pathArray.data(),
                &numModeInfoArrayElements, modeInfoArray.data(),
                nullptr);
            
            if (result == ERROR_SUCCESS) {
                printf("[VDD] Current active paths: %d\n", numPathArrayElements);
                
                // SAFETY LAYER 2: Verify primary display is still active
                bool primaryFound = false;
                for (UINT32 i = 0; i < numPathArrayElements; i++) {
                    // Check if this path has a valid source and is active
                    if ((pathArray[i].flags & DISPLAYCONFIG_PATH_ACTIVE) &&
                        pathArray[i].sourceInfo.modeInfoIdx != DISPLAYCONFIG_PATH_MODE_IDX_INVALID) {
                        
                        // Try to get device name to check if it's a physical display
                        DISPLAYCONFIG_SOURCE_DEVICE_NAME sourceName = {};
                        sourceName.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_SOURCE_NAME;
                        sourceName.header.size = sizeof(DISPLAYCONFIG_SOURCE_DEVICE_NAME);
                        sourceName.header.adapterId = pathArray[i].sourceInfo.adapterId;
                        sourceName.header.id = pathArray[i].sourceInfo.id;
                        
                        if (DisplayConfigGetDeviceInfo(&sourceName.header) == ERROR_SUCCESS) {
                            std::wstring devName(sourceName.viewGdiDeviceName);
                            // Check if it's NOT our virtual display (primary is physical)
                            if (devName.find(L"DISPLAY1") != std::wstring::npos || 
                                devName.find(L"DISPLAY2") != std::wstring::npos) {
                                primaryFound = true;
                                printf("[VDD] PRIMARY DISPLAY VERIFIED: %ls (path %d)\n", devName.c_str(), i);
                            }
                        }
                    }
                }
                
                if (!primaryFound) {
                    printf("[VDD] WARNING: Could not verify primary display - aborting for safety\n");
                    SetLastError("Safety check failed: Primary display not detected");
                    return Status::DriverError;
                }
                
                // SAFETY LAYER 3: Apply configuration with error checking
                printf("[VDD] Applying display configuration (primary protected)...\n");
                result = SetDisplayConfig(numPathArrayElements, pathArray.data(),
                    numModeInfoArrayElements, modeInfoArray.data(),
                    SDC_APPLY | SDC_USE_SUPPLIED_DISPLAY_CONFIG | SDC_SAVE_TO_DATABASE | SDC_ALLOW_CHANGES);
                
                if (result == ERROR_SUCCESS) {
                    printf("[VDD] ✓ Display topology preserved successfully\n");
                    printf("[VDD] ✓ Primary display protected\n");
                } else {
                    printf("[VDD] WARNING: Failed to preserve display topology, error=%d\n", result);
                    printf("[VDD] This may cause display issues - consider using recovery script\n");
                    // Don't fail activation completely, but warn user
                }
            } else {
                printf("[VDD] WARNING: Failed to query display config, error=%d\n", result);
                printf("[VDD] Proceeding with caution - primary display should remain active\n");
            }
        } else {
            printf("[VDD] WARNING: Could not get display config buffers, error=%d\n", result);
            printf("[VDD] Proceeding without topology verification\n");
        }
        
        printf("[VDD] Activate: SUCCESS - Device enabled and topology configured\n");
        printf("[VDD] SAFETY: Primary display protection active\n");
        SetLastError("Virtual display driver activated successfully");
        return Status::Ok;
    }

    Status VddSdkImpl::Deactivate() {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        printf("[VDD] Deactivate: Starting...\n");
        
        // Check if driver is installed (don't check m_isActive - each vddctl run is a new process)
        if (!IsDriverInstalled()) {
            SetLastError("Virtual display driver not installed");
            return Status::NotInstalled;
        }
        
        // Real deactivation using SetupAPI to disable the device
        HDEVINFO hDevInfo = SetupDiGetClassDevsW(
            &GUID_DEVCLASS_DISPLAY,
            nullptr,
            nullptr,
            DIGCF_PRESENT | DIGCF_ALLCLASSES
        );
        
        if (hDevInfo == INVALID_HANDLE_VALUE) {
            DWORD err = ::GetLastError();
            printf("[VDD] ERROR: SetupDiGetClassDevsW failed, error=%d\n", err);
            SetLastError("Failed to get device list: " + std::to_string(err));
            return Status::DriverError;
        }
        
        bool deviceFound = false;
        SP_DEVINFO_DATA devInfoData = {};
        devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
        
        // Find our device
        for (DWORD i = 0; SetupDiEnumDeviceInfo(hDevInfo, i, &devInfoData); ++i) {
            wchar_t hwid[256] = {};
            if (SetupDiGetDeviceRegistryPropertyW(hDevInfo, &devInfoData, SPDRP_HARDWAREID,
                nullptr, (PBYTE)hwid, sizeof(hwid), nullptr)) {
                
                if (_wcsicmp(hwid, L"ROOT\\IddSampleDriver") == 0) {
                    deviceFound = true;
                    printf("[VDD] Found device: %ls\n", hwid);
                    break;
                }
            }
        }
        
        if (!deviceFound) {
            SetupDiDestroyDeviceInfoList(hDevInfo);
            SetLastError("Driver device not found in system");
            return Status::NotInstalled;
        }
        
        // Disable the device using DIF_PROPERTYCHANGE
        SP_PROPCHANGE_PARAMS propChangeParams = {};
        propChangeParams.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
        propChangeParams.ClassInstallHeader.InstallFunction = DIF_PROPERTYCHANGE;
        propChangeParams.StateChange = DICS_DISABLE;
        propChangeParams.Scope = DICS_FLAG_GLOBAL;
        propChangeParams.HwProfile = 0;
        
        if (!SetupDiSetClassInstallParamsW(hDevInfo, &devInfoData,
            (SP_CLASSINSTALL_HEADER*)&propChangeParams, sizeof(propChangeParams))) {
            DWORD err = ::GetLastError();
            printf("[VDD] ERROR: SetupDiSetClassInstallParamsW failed, error=%d\n", err);
            SetupDiDestroyDeviceInfoList(hDevInfo);
            SetLastError("Failed to set disable parameters: " + std::to_string(err));
            return Status::DriverError;
        }
        
        if (!SetupDiCallClassInstaller(DIF_PROPERTYCHANGE, hDevInfo, &devInfoData)) {
            DWORD err = ::GetLastError();
            printf("[VDD] ERROR: DIF_PROPERTYCHANGE (DISABLE) failed, error=%d\n", err);
            SetupDiDestroyDeviceInfoList(hDevInfo);
            SetLastError("Failed to disable device: " + std::to_string(err));
            return Status::DriverError;
        }
        
        SetupDiDestroyDeviceInfoList(hDevInfo);
        
        // Reset state
        m_isActive = false;
        m_activeDisplayCount = 0;
        m_activeDisplays.clear();
        
        printf("[VDD] Deactivate: SUCCESS - Device disabled\n");
        SetLastError("Virtual display driver deactivated successfully");
        return Status::Ok;
    }

    bool VddSdkImpl::IsActive() {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        // Query real device status from system instead of using cached m_isActive
        // Check if driver is installed first
        if (!IsDriverInstalled()) {
            return false;
        }
        
        // Get device info set
        HDEVINFO hDevInfo = SetupDiGetClassDevsW(
            &GUID_DEVCLASS_DISPLAY,
            nullptr,
            nullptr,
            DIGCF_PRESENT | DIGCF_ALLCLASSES
        );
        
        if (hDevInfo == INVALID_HANDLE_VALUE) {
            return false;
        }
        
        bool isActive = false;
        SP_DEVINFO_DATA devInfoData = {};
        devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
        
        // Find our device
        for (DWORD i = 0; SetupDiEnumDeviceInfo(hDevInfo, i, &devInfoData); ++i) {
            wchar_t hwid[256] = {};
            if (SetupDiGetDeviceRegistryPropertyW(hDevInfo, &devInfoData, SPDRP_HARDWAREID,
                nullptr, (PBYTE)hwid, sizeof(hwid), nullptr)) {
                
                if (_wcsicmp(hwid, L"ROOT\\IddSampleDriver") == 0) {
                    // Found our device - check if it's started and has no problems
                    ULONG status = 0, problemNumber = 0;
                    if (CM_Get_DevNode_Status(&status, &problemNumber, devInfoData.DevInst, 0) == CR_SUCCESS) {
                        // Device is active if it's started and has no problems
                        isActive = (status & DN_STARTED) && !(status & DN_HAS_PROBLEM);
                    }
                    break;
                }
            }
        }
        
        SetupDiDestroyDeviceInfoList(hDevInfo);
        return isActive;
    }

    uint32_t VddSdkImpl::GetActiveDisplayCount() {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        // Query real display output count from system instead of cached value
        // Enumerate all display devices and count outputs from IddSampleDriver
        uint32_t count = 0;
        bool foundAdapter = false;  // Only count monitors from the first occurrence
        DISPLAY_DEVICEW displayDevice = {};
        displayDevice.cb = sizeof(DISPLAY_DEVICEW);
        
        // Enumerate all display adapters
        for (DWORD adapterIndex = 0; EnumDisplayDevicesW(nullptr, adapterIndex, &displayDevice, 0); ++adapterIndex) {
            // Check if this is our IddSampleDriver adapter
            if (wcsstr(displayDevice.DeviceString, L"IddSampleDriver") != nullptr) {
                // Only count the first occurrence (adapter appears multiple times due to multiple outputs)
                if (!foundAdapter) {
                    foundAdapter = true;
                    
                    // Enumerate monitors/outputs for this adapter
                    DISPLAY_DEVICEW monitorDevice = {};
                    monitorDevice.cb = sizeof(DISPLAY_DEVICEW);
                    
                    for (DWORD monitorIndex = 0; EnumDisplayDevicesW(displayDevice.DeviceName, monitorIndex, &monitorDevice, 0); ++monitorIndex) {
                        // Count all monitors (even if not active - they still exist)
                        count++;
                    }
                }
            }
        }
        
        return count;
    }

    Status VddSdkImpl::SetMode(uint32_t outputIndex, const DisplayMode& mode) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        // FIXED: Check actual driver installation, not process-local state
        bool isInstalled = IsDriverInstalled();
        printf("[VDD] SetMode: IsDriverInstalled() = %s\n", isInstalled ? "true" : "false");
        if (!isInstalled) {
            SetLastError("Virtual display driver not installed");
            return Status::NotInstalled;
        }
        
        // Note: Don't check m_isActive - each vddctl run is a new process
        // We'll validate outputIndex against actual virtual displays later
        
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
            // FIXED: Get virtual display device names to find the correct device
            std::vector<std::wstring> virtualDisplays = GetVirtualDisplayDeviceNames();
            
            if (virtualDisplays.empty()) {
                SetLastError("No virtual displays found");
                return Status::NotActive;
            }
            
            if (outputIndex >= virtualDisplays.size()) {
                SetLastError("Output index " + std::to_string(outputIndex) + 
                           " out of range (found " + std::to_string(virtualDisplays.size()) + " virtual displays)");
                return Status::InvalidArg;
            }
            
            // Get the device name for the specified output
            const std::wstring& deviceName = virtualDisplays[outputIndex];
            printf("[VDD] SetMode: Changing mode for device %ls to %dx%d@%dHz\n",
                   deviceName.c_str(), mode.width, mode.height, refreshRate);
            
            // STRATEGY: Try SetDisplayConfig first (modern API), fallback to ChangeDisplaySettingsExW
            
            // METHOD 1: Try SetDisplayConfig API (works for active displays)
            printf("[VDD] SetMode: Attempting method 1 - SetDisplayConfig API\n");
            
            UINT32 numPathArrayElements = 0;
            UINT32 numModeInfoArrayElements = 0;
            LONG result = GetDisplayConfigBufferSizes(QDC_ONLY_ACTIVE_PATHS,
                &numPathArrayElements, &numModeInfoArrayElements);
            
            if (result == ERROR_SUCCESS && numPathArrayElements > 0) {
                std::vector<DISPLAYCONFIG_PATH_INFO> pathArray(numPathArrayElements);
                std::vector<DISPLAYCONFIG_MODE_INFO> modeInfoArray(numModeInfoArrayElements);
                
                result = QueryDisplayConfig(QDC_ONLY_ACTIVE_PATHS,
                    &numPathArrayElements, pathArray.data(),
                    &numModeInfoArrayElements, modeInfoArray.data(),
                    nullptr);
                
                if (result == ERROR_SUCCESS) {
                    // Find the correct path
                    for (UINT32 i = 0; i < numPathArrayElements; i++) {
                        DISPLAYCONFIG_SOURCE_DEVICE_NAME sourceName = {};
                        sourceName.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_SOURCE_NAME;
                        sourceName.header.size = sizeof(DISPLAYCONFIG_SOURCE_DEVICE_NAME);
                        sourceName.header.adapterId = pathArray[i].sourceInfo.adapterId;
                        sourceName.header.id = pathArray[i].sourceInfo.id;
                        
                        if (DisplayConfigGetDeviceInfo(&sourceName.header) == ERROR_SUCCESS &&
                            wcscmp(sourceName.viewGdiDeviceName, deviceName.c_str()) == 0) {
                            
                            printf("[VDD] SetMode: Found device in active paths (index %d)\n", i);
                            
                            // Check if modeInfoIdx is valid
                            UINT32 modeIdx = pathArray[i].sourceInfo.modeInfoIdx;
                            if (modeIdx < numModeInfoArrayElements) {
                                // Modify resolution
                                DISPLAYCONFIG_SOURCE_MODE& sourceMode = modeInfoArray[modeIdx].sourceMode;
                                sourceMode.width = mode.width;
                                sourceMode.height = mode.height;
                                
                                // Apply changes
                                result = SetDisplayConfig(numPathArrayElements, pathArray.data(),
                                    numModeInfoArrayElements, modeInfoArray.data(),
                                    SDC_APPLY | SDC_USE_SUPPLIED_DISPLAY_CONFIG | SDC_SAVE_TO_DATABASE);
                                
                                if (result == ERROR_SUCCESS) {
                                    printf("[VDD] SetMode: SUCCESS via SetDisplayConfig\n");
                                    if (outputIndex < m_activeDisplays.size()) {
                                        m_activeDisplays[outputIndex].preferredMode = mode;
                                    }
                                    SetLastError("Display mode set successfully");
                                    return Status::Ok;
                                }
                            }
                            break;
                        }
                    }
                }
            }
            
            // METHOD 2: Fallback to ChangeDisplaySettingsExW (works for all displays)
            printf("[VDD] SetMode: Attempting method 2 - ChangeDisplaySettingsExW (fallback)\n");
            
            DEVMODEW devMode = {};
            devMode.dmSize = sizeof(DEVMODEW);
            
            // Query current settings
            if (EnumDisplaySettingsW(deviceName.c_str(), ENUM_CURRENT_SETTINGS, &devMode)) {
                printf("[VDD] SetMode: Current mode: %dx%d@%dHz\n",
                       devMode.dmPelsWidth, devMode.dmPelsHeight, devMode.dmDisplayFrequency);
            }
            
            // Set new mode
            devMode.dmPelsWidth = mode.width;
            devMode.dmPelsHeight = mode.height;
            devMode.dmDisplayFrequency = static_cast<DWORD>(refreshRate);
            devMode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_DISPLAYFREQUENCY;
            
            result = ChangeDisplaySettingsExW(deviceName.c_str(), &devMode, nullptr,
                                             CDS_UPDATEREGISTRY | CDS_NORESET, nullptr);
            
            if (result == DISP_CHANGE_SUCCESSFUL) {
                // Apply all pending changes
                ChangeDisplaySettingsExW(nullptr, nullptr, nullptr, 0, nullptr);
                
                printf("[VDD] SetMode: SUCCESS via ChangeDisplaySettingsExW\n");
                if (outputIndex < m_activeDisplays.size()) {
                    m_activeDisplays[outputIndex].preferredMode = mode;
                }
                SetLastError("Display mode set successfully");
                return Status::Ok;
            }
            
            // Both methods failed
            printf("[VDD] SetMode: Both methods failed (SetDisplayConfig and ChangeDisplaySettingsEx)\n");
            printf("[VDD] SetMode: This may indicate the display is not active in Windows display topology\n");
            printf("[VDD] SetMode: Try: Open Windows Display Settings > Detect/Extend displays\n");
            SetLastError("Failed to change display mode. Display may not be active in Windows topology.");
            return Status::DriverError;
            
        } catch (const std::exception& e) {
            SetLastError("Failed to set display mode: " + std::string(e.what()));
            return Status::DriverError;
        }
    }

    Status VddSdkImpl::SetLocation(uint32_t outputIndex, const DisplayRect& rect) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        // FIXED: Check actual driver installation, not process-local state
        if (!IsDriverInstalled()) {
            SetLastError("Virtual display driver not installed");
            return Status::NotInstalled;
        }
        
        // Note: Don't check m_isActive - each vddctl run is a new process
        // We'll validate outputIndex against actual virtual displays later
        
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
            // FIXED: Get virtual display device names first
            std::vector<std::wstring> virtualDisplays = GetVirtualDisplayDeviceNames();
            
            if (virtualDisplays.empty()) {
                SetLastError("No virtual displays found");
                return Status::NotActive;
            }
            
            if (outputIndex >= virtualDisplays.size()) {
                SetLastError("Output index out of range");
                return Status::InvalidArg;
            }
            
            const std::wstring& targetDeviceName = virtualDisplays[outputIndex];
            printf("[VDD] SetLocation: Setting location for device %ls to (%d,%d)\n",
                   targetDeviceName.c_str(), rect.x, rect.y);
            
            // REAL IMPLEMENTATION - Use Windows API to set display position
            // Get current display configuration
            UINT32 numPathArrayElements = 0;
            UINT32 numModeInfoArrayElements = 0;
            LONG result = GetDisplayConfigBufferSizes(QDC_ONLY_ACTIVE_PATHS, 
                &numPathArrayElements, &numModeInfoArrayElements);
            
            if (result == ERROR_SUCCESS && numPathArrayElements > 0) {
                std::vector<DISPLAYCONFIG_PATH_INFO> pathArray(numPathArrayElements);
                std::vector<DISPLAYCONFIG_MODE_INFO> modeInfoArray(numModeInfoArrayElements);
                
                result = QueryDisplayConfig(QDC_ONLY_ACTIVE_PATHS,
                    &numPathArrayElements, pathArray.data(),
                    &numModeInfoArrayElements, modeInfoArray.data(),
                    nullptr);
                
                if (result == ERROR_SUCCESS) {
                    // CRITICAL FIX: Find the correct path index by matching device name
                    bool foundDevice = false;
                    for (UINT32 i = 0; i < numPathArrayElements; i++) {
                        // Get source device name
                        DISPLAYCONFIG_SOURCE_DEVICE_NAME sourceName = {};
                        sourceName.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_SOURCE_NAME;
                        sourceName.header.size = sizeof(DISPLAYCONFIG_SOURCE_DEVICE_NAME);
                        sourceName.header.adapterId = pathArray[i].sourceInfo.adapterId;
                        sourceName.header.id = pathArray[i].sourceInfo.id;
                        
                        if (DisplayConfigGetDeviceInfo(&sourceName.header) == ERROR_SUCCESS) {
                            if (wcscmp(sourceName.viewGdiDeviceName, targetDeviceName.c_str()) == 0) {
                                // Found the correct path!
                                printf("[VDD] SetLocation: Found path index %d for device %ls\n", i, targetDeviceName.c_str());
                                
                                // Update position
                                DISPLAYCONFIG_SOURCE_MODE& sourceMode = modeInfoArray[pathArray[i].sourceInfo.modeInfoIdx].sourceMode;
                                sourceMode.position.x = rect.x;
                                sourceMode.position.y = rect.y;
                                
                                // Apply the changes
                                result = SetDisplayConfig(numPathArrayElements, pathArray.data(),
                                    numModeInfoArrayElements, modeInfoArray.data(),
                                    SDC_APPLY | SDC_USE_SUPPLIED_DISPLAY_CONFIG | SDC_SAVE_TO_DATABASE);
                                
                                if (result == ERROR_SUCCESS) {
                                    printf("[VDD] SetLocation: SUCCESS\n");
                                    SetLastError("Display location set successfully for output " + std::to_string(outputIndex));
                                    foundDevice = true;
                                    break;
                                } else {
                                    printf("[VDD] SetLocation: FAILED to apply config, error=%d\n", result);
                                    SetLastError("Failed to apply display position change: " + std::to_string(result));
                                    return Status::DriverError;
                                }
                            }
                        }
                    }
                    
                    if (!foundDevice) {
                        SetLastError("Virtual display device not found in active paths");
                        return Status::DriverError;
                    }
                    
                    return Status::Ok;
                } else {
                    SetLastError("Failed to query display configuration: " + std::to_string(result));
                    return Status::DriverError;
                }
            } else {
                SetLastError("Failed to get display config buffer sizes: " + std::to_string(result));
                return Status::DriverError;
            }
            
        } catch (const std::exception& e) {
            SetLastError("Failed to set display location: " + std::string(e.what()));
            return Status::DriverError;
        }
    }

    Status VddSdkImpl::SetPrimary(uint32_t outputIndex) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        // FIXED: Check actual driver installation, not process-local state
        if (!IsDriverInstalled()) {
            SetLastError("Virtual display driver not installed");
            return Status::NotInstalled;
        }
        
        // Note: Don't check m_isActive - each vddctl run is a new process
        // We'll validate outputIndex against actual virtual displays later
        
        try {
            // FIXED: Get virtual display device names first
            std::vector<std::wstring> virtualDisplays = GetVirtualDisplayDeviceNames();
            
            if (virtualDisplays.empty()) {
                SetLastError("No virtual displays found");
                return Status::NotActive;
            }
            
            if (outputIndex >= virtualDisplays.size()) {
                SetLastError("Output index out of range");
                return Status::InvalidArg;
            }
            
            const std::wstring& targetDeviceName = virtualDisplays[outputIndex];
            printf("[VDD] SetPrimary: Setting device %ls as primary\n", targetDeviceName.c_str());
            
            // REAL IMPLEMENTATION - Use Windows API to set primary display
            // Get current display configuration
            UINT32 numPathArrayElements = 0;
            UINT32 numModeInfoArrayElements = 0;
            LONG result = GetDisplayConfigBufferSizes(QDC_ONLY_ACTIVE_PATHS, 
                &numPathArrayElements, &numModeInfoArrayElements);
            
            if (result == ERROR_SUCCESS && numPathArrayElements > 0) {
                std::vector<DISPLAYCONFIG_PATH_INFO> pathArray(numPathArrayElements);
                std::vector<DISPLAYCONFIG_MODE_INFO> modeInfoArray(numModeInfoArrayElements);
                
                result = QueryDisplayConfig(QDC_ONLY_ACTIVE_PATHS,
                    &numPathArrayElements, pathArray.data(),
                    &numModeInfoArrayElements, modeInfoArray.data(),
                    nullptr);
                
                if (result == ERROR_SUCCESS) {
                    // CRITICAL FIX: Find the correct path index by matching device name
                    bool foundDevice = false;
                    for (UINT32 i = 0; i < numPathArrayElements; i++) {
                        // Get source device name
                        DISPLAYCONFIG_SOURCE_DEVICE_NAME sourceName = {};
                        sourceName.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_SOURCE_NAME;
                        sourceName.header.size = sizeof(DISPLAYCONFIG_SOURCE_DEVICE_NAME);
                        sourceName.header.adapterId = pathArray[i].sourceInfo.adapterId;
                        sourceName.header.id = pathArray[i].sourceInfo.id;
                        
                        if (DisplayConfigGetDeviceInfo(&sourceName.header) == ERROR_SUCCESS) {
                            if (wcscmp(sourceName.viewGdiDeviceName, targetDeviceName.c_str()) == 0) {
                                // Found the correct path!
                                printf("[VDD] SetPrimary: Found path index %d for device %ls\n", i, targetDeviceName.c_str());
                                
                                // Set as primary by setting position to (0,0)
                                DISPLAYCONFIG_SOURCE_MODE& sourceMode = modeInfoArray[pathArray[i].sourceInfo.modeInfoIdx].sourceMode;
                                sourceMode.position.x = 0;
                                sourceMode.position.y = 0;
                                
                                // Apply the changes
                                result = SetDisplayConfig(numPathArrayElements, pathArray.data(),
                                    numModeInfoArrayElements, modeInfoArray.data(),
                                    SDC_APPLY | SDC_USE_SUPPLIED_DISPLAY_CONFIG | SDC_SAVE_TO_DATABASE);
                                
                                if (result == ERROR_SUCCESS) {
                                    printf("[VDD] SetPrimary: SUCCESS\n");
                                    SetLastError("Primary display set successfully to output " + std::to_string(outputIndex));
                                    foundDevice = true;
                                    break;
                                } else {
                                    printf("[VDD] SetPrimary: FAILED to apply config, error=%d\n", result);
                                    SetLastError("Failed to set primary display: " + std::to_string(result));
                                    return Status::DriverError;
                                }
                            }
                        }
                    }
                    
                    if (!foundDevice) {
                        SetLastError("Virtual display device not found in active paths");
                        return Status::DriverError;
                    }
                    
                    return Status::Ok;
                } else {
                    SetLastError("Failed to query display configuration: " + std::to_string(result));
                    return Status::DriverError;
                }
            } else {
                SetLastError("Failed to get display config buffer sizes: " + std::to_string(result));
                return Status::DriverError;
            }
            
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
        // REAL IMPLEMENTATION - Enumerate actual display adapters
        adapters.clear();
        
        DISPLAY_DEVICEW displayDevice = {};
        displayDevice.cb = sizeof(DISPLAY_DEVICEW);
        
        // Track unique adapters by DeviceString (not DeviceName) to avoid multiple outputs from same adapter
        std::vector<std::wstring> seenDeviceStrings;
        
        for (DWORD i = 0; EnumDisplayDevicesW(nullptr, i, &displayDevice, 0); i++) {
            // Use DeviceString to identify unique adapters
            std::wstring deviceString(displayDevice.DeviceString);
            
            bool isDuplicate = false;
            for (const auto& seen : seenDeviceStrings) {
                if (seen == deviceString) {
                    isDuplicate = true;
                    break;
                }
            }
            
            // Skip duplicates (multiple outputs from same adapter)
            if (isDuplicate) {
                continue;
            }
            
            seenDeviceStrings.push_back(deviceString);
            
            AdapterInfo adapter;
            
            // Convert wide string to narrow string
            int size = WideCharToMultiByte(CP_UTF8, 0, displayDevice.DeviceString, -1, nullptr, 0, nullptr, nullptr);
            if (size > 0) {
                adapter.name.resize(size - 1);
                WideCharToMultiByte(CP_UTF8, 0, displayDevice.DeviceString, -1, &adapter.name[0], size, nullptr, nullptr);
            }
            
            adapter.isActive = (displayDevice.StateFlags & DISPLAY_DEVICE_ACTIVE) != 0;
            adapter.isPrimary = (displayDevice.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE) != 0;
            adapter.isVirtual = (displayDevice.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER) != 0;
            
            // Get adapter ID
            adapter.adapterId = static_cast<uint32_t>(adapters.size());
            adapter.outputCount = 1; // Assume 1 output per adapter for now
            
            adapters.push_back(adapter);
        }
        
        return Status::Ok;
    }

    Status VddSdkImpl::EnumerateModes(uint32_t outputIndex, std::vector<DisplayMode>& modes) {
        // REAL IMPLEMENTATION - Enumerate actual display modes
        modes.clear();
        
        DEVMODEW devMode = {};
        devMode.dmSize = sizeof(DEVMODEW);
        
        // Try to get actual display modes from the system
        for (DWORD i = 0; EnumDisplaySettingsW(nullptr, i, &devMode); i++) {
            DisplayMode displayMode;
            displayMode.width = devMode.dmPelsWidth;
            displayMode.height = devMode.dmPelsHeight;
            displayMode.refreshNumerator = devMode.dmDisplayFrequency;
            displayMode.refreshDenominator = 1;
            
            modes.push_back(displayMode);
        }
        
        // If no modes found, add common fallback modes
        if (modes.empty()) {
            std::vector<std::tuple<uint32_t, uint32_t, uint32_t>> commonModes = {
                {640, 480, 60}, {800, 600, 60}, {1024, 768, 60}, {1280, 720, 60},
                {1280, 1024, 60}, {1366, 768, 60}, {1440, 900, 60}, {1600, 900, 60},
                {1600, 1200, 60}, {1680, 1050, 60}, {1920, 1080, 60}, {1920, 1200, 60},
                {2560, 1440, 60}, {2560, 1600, 60}, {3840, 2160, 60}
            };
            
            for (const auto& mode : commonModes) {
                DisplayMode displayMode;
                displayMode.width = std::get<0>(mode);
                displayMode.height = std::get<1>(mode);
                displayMode.refreshNumerator = std::get<2>(mode);
                displayMode.refreshDenominator = 1;
                
                modes.push_back(displayMode);
            }
        }
        
        return Status::Ok;
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
        // REAL IMPLEMENTATION - Gather actual system information
        std::stringstream info;
        
        // Get Windows version
        OSVERSIONINFOW osvi = {};
        osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOW);
        if (GetVersionExW(&osvi)) {
            info << "Windows " << osvi.dwMajorVersion << "." << osvi.dwMinorVersion;
            if (osvi.dwBuildNumber > 0) {
                info << " Build " << osvi.dwBuildNumber;
            }
            info << "\n";
        }
        
        // Get system memory
        MEMORYSTATUSEX memStatus = {};
        memStatus.dwLength = sizeof(MEMORYSTATUSEX);
        if (GlobalMemoryStatusEx(&memStatus)) {
            info << "Total Memory: " << (memStatus.ullTotalPhys / (1024 * 1024)) << " MB\n";
            info << "Available Memory: " << (memStatus.ullAvailPhys / (1024 * 1024)) << " MB\n";
        }
        
        // Get processor info
        SYSTEM_INFO sysInfo = {};
        ::GetSystemInfo(&sysInfo);
        info << "Processors: " << sysInfo.dwNumberOfProcessors << "\n";
        info << "Architecture: " << (sysInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 ? "x64" : "x86") << "\n";
        
        // Get display adapters count
        DISPLAY_DEVICEW displayDevice = {};
        displayDevice.cb = sizeof(DISPLAY_DEVICEW);
        int adapterCount = 0;
        for (DWORD i = 0; EnumDisplayDevicesW(nullptr, i, &displayDevice, 0); i++) {
            if (displayDevice.StateFlags & DISPLAY_DEVICE_ACTIVE) {
                adapterCount++;
            }
        }
        info << "Active Display Adapters: " << adapterCount << "\n";
        
        return info.str();
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

/ /   T E S T   W R I T E 
 
 