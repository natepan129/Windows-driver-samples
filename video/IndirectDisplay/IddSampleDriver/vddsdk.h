#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <cstdint>

// Export macro - 必須在函數聲明之前定義
#ifdef VDD_EXPORTS
#define VDD_API __declspec(dllexport)
#else
#define VDD_API __declspec(dllimport)
#endif

namespace vdd {

struct Version { 
    int major, minor, patch; 
};

enum class Status {
    Ok, AlreadyInstalled, NotInstalled, NotActive, Busy,
    AdminRequired, DriverError, Timeout, InvalidArg, OsUnsupported
};

struct DisplayMode { 
    uint32_t width, height; 
    uint32_t refreshNumerator, refreshDenominator; 
};

struct DisplayRect { 
    int32_t x, y; 
    uint32_t width, height; 
};

struct VirtualDisplayDesc {
    std::string name;            // Friendly name reported via EDID
    DisplayMode preferredMode;   // Initial mode
    bool hdr10 = false;          // Optional: EOTF flags in EDID
    bool stereoscopic = false;   // Optional: 3D stereo support
};

struct AdapterInfo {
    std::wstring deviceInstanceId; // PnP ID
    bool isVirtual;                 // true for VDD
    bool isActive;                  // enumerated & enabled
    std::string name;               // friendly name
};

// Basic SDK functions for testing Driver.cpp IOCTL functionality
extern "C" {

// Initialization / versioning
VDD_API Version GetVersion();
VDD_API const char* GetStatusString(Status status);

// Driver lifecycle (simplified for testing)
VDD_API Status InstallDriver(const wchar_t* infPath);
VDD_API Status UninstallDriver();

// Runtime control (using IOCTL to communicate with Driver.cpp)
VDD_API Status Activate(const VirtualDisplayDesc& desc, uint32_t count = 1);
VDD_API Status Deactivate();

// Configuration (direct IOCTL calls to Driver.cpp)
VDD_API Status SetMode(uint32_t outputIndex, const DisplayMode& mode);
VDD_API Status SetLocation(uint32_t outputIndex, const DisplayRect& rect);
VDD_API Status SetPrimary(uint32_t outputIndex);

// Query functions
VDD_API Status EnumerateAdapters(std::vector<AdapterInfo>& out);
VDD_API Status EnumerateModes(uint32_t outputIndex, std::vector<DisplayMode>& modes);

// Test functions for Driver.cpp IOCTL
VDD_API Status OpenDevice();
VDD_API void CloseDevice();
VDD_API Status TestDriverConnection();
VDD_API Status ConfigureDisplay(uint32_t index, uint32_t width, uint32_t height, uint32_t refresh, const char* name);
VDD_API Status SetMonitorCount(uint32_t count);

}

} // namespace vdd

