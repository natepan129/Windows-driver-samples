/*++

Copyright (c) 2025 VDD SDK Project

Abstract:

    Virtual Display Driver (VDD) SDK - Usage Example
    This file demonstrates how to use the VDD SDK API.

Environment:

    User Mode, C++17

--*/

#include "vddsdk.h"
#include <iostream>
#include <thread>
#include <chrono>

using namespace vdd;

void PrintStatus(const std::string& operation, Status status) {
    std::cout << operation << ": " << StatusToString(status);
    if (status != Status::Ok) {
        std::cout << " (" << GetLastError() << ")";
    }
    std::cout << std::endl;
}

void PrintVersion() {
    Version version = GetVersion();
    std::cout << "VDD SDK Version: " << version.major << "." 
              << version.minor << "." << version.patch << std::endl;
}

void PrintSystemInfo() {
    std::cout << "System Info:" << std::endl;
    std::cout << GetSystemInfo() << std::endl;
}

void PrintAdapters() {
    std::vector<AdapterInfo> adapters;
    Status status = EnumerateAdapters(adapters);
    
    if (status == Status::Ok) {
        std::cout << "Found " << adapters.size() << " display adapters:" << std::endl;
        for (size_t i = 0; i < adapters.size(); ++i) {
            const auto& adapter = adapters[i];
            std::cout << "  [" << i << "] " << adapter.name;
            if (adapter.isVirtual) {
                std::cout << " (Virtual)";
            }
            if (adapter.isActive) {
                std::cout << " (Active)";
            }
            if (adapter.isPrimary) {
                std::cout << " (Primary)";
            }
            std::cout << std::endl;
        }
    } else {
        PrintStatus("EnumerateAdapters", status);
    }
}

void PrintModes(uint32_t outputIndex) {
    std::vector<DisplayMode> modes;
    Status status = EnumerateModes(outputIndex, modes);
    
    if (status == Status::Ok) {
        std::cout << "Supported modes for output " << outputIndex << ":" << std::endl;
        for (size_t i = 0; i < modes.size(); ++i) {
            const auto& mode = modes[i];
            std::cout << "  [" << i << "] " << mode.width << "x" << mode.height 
                      << "@" << mode.refreshNumerator << "/" << mode.refreshDenominator << std::endl;
        }
    } else {
        PrintStatus("EnumerateModes", status);
    }
}

void BasicUsageExample() {
    std::cout << "\n=== Basic Usage Example ===" << std::endl;
    
    // Initialize SDK
    SdkConfig config;
    config.enableLogging = true;
    config.logFilePath = L"C:\\temp\\vdd.log";
    
    PrintStatus("Initialize", Initialize(config));
    PrintVersion();
    PrintSystemInfo();
    
    // Check if driver is installed
    if (IsDriverInstalled()) {
        std::cout << "VDD Driver is installed" << std::endl;
        Version driverVersion = GetDriverVersion();
        std::cout << "Driver Version: " << driverVersion.major << "." 
                  << driverVersion.minor << "." << driverVersion.patch << std::endl;
    } else {
        std::cout << "VDD Driver is not installed" << std::endl;
    }
    
    // Enumerate current adapters
    PrintAdapters();
    
    // Shutdown
    PrintStatus("Shutdown", Shutdown());
}

void VirtualDisplayExample() {
    std::cout << "\n=== Virtual Display Example ===" << std::endl;
    
    // Initialize SDK
    PrintStatus("Initialize", Initialize());
    
    // Create virtual display description
    VirtualDisplayDesc desc;
    desc.name = "VDD Example Display";
    desc.preferredMode = { 1920, 1080, 60, 1 };
    desc.hdr10 = false;
    desc.stereoscopic = false;
    desc.manufacturer = "VDD SDK";
    desc.model = "Example Display";
    
    // Activate virtual display
    PrintStatus("Activate", Activate(desc, 1));
    
    if (IsActive()) {
        std::cout << "Virtual display is active" << std::endl;
        std::cout << "Active display count: " << GetActiveDisplayCount() << std::endl;
        
        // Configure display
        DisplayRect rect = { 1920, 0, 1920, 1080 }; // Position to the right of main display
        PrintStatus("SetLocation", SetLocation(0, rect));
        
        // Set as primary (optional)
        PrintStatus("SetPrimary", SetPrimary(0));
        
        // Get current mode
        DisplayMode currentMode;
        if (GetMode(0, currentMode) == Status::Ok) {
            std::cout << "Current mode: " << currentMode.width << "x" << currentMode.height 
                      << "@" << currentMode.refreshNumerator << "/" << currentMode.refreshDenominator << std::endl;
        }
        
        // Enumerate supported modes
        PrintModes(0);
        
        // Wait a bit
        std::cout << "Virtual display active for 5 seconds..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(5));
        
        // Deactivate
        PrintStatus("Deactivate", Deactivate());
    }
    
    // Shutdown
    PrintStatus("Shutdown", Shutdown());
}

void SessionManagementExample() {
    std::cout << "\n=== Session Management Example ===" << std::endl;
    
    // Initialize SDK
    PrintStatus("Initialize", Initialize());
    
    // Begin session
    LeaseHandle lease;
    PrintStatus("BeginSession", BeginSession(lease));
    
    if (lease.IsValid()) {
        std::cout << "Session created with lease ID: " << lease.id << std::endl;
        
        // Activate with session management
        ActivateOptions options;
        options.desc.name = "VDD Session Display";
        options.desc.preferredMode = { 2560, 1440, 90, 1 };
        options.count = 1;
        options.autoRestoreOnCrash = true;
        options.heartbeatIntervalMs = 1000;
        options.leaseTimeoutMs = 30000;
        
        PrintStatus("ActivateLeased", ActivateLeased(options, lease));
        
        if (IsActive()) {
            std::cout << "Session-managed virtual display is active" << std::endl;
            
            // Send heartbeats
            for (int i = 0; i < 10; ++i) {
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                
                bool isActive;
                uint32_t timeRemaining;
                Status status = GetSessionState(lease, isActive, timeRemaining);
                if (status == Status::Ok) {
                    std::cout << "Session active: " << (isActive ? "Yes" : "No") 
                              << ", Time remaining: " << timeRemaining << "ms" << std::endl;
                }
                
                // Send heartbeat
                PrintStatus("Heartbeat", Heartbeat(lease));
            }
            
            // End session
            PrintStatus("EndSession", EndSession(lease));
        }
    }
    
    // Shutdown
    PrintStatus("Shutdown", Shutdown());
}

void AdvancedFeaturesExample() {
    std::cout << "\n=== Advanced Features Example ===" << std::endl;
    
    // Initialize SDK
    PrintStatus("Initialize", Initialize());
    
    // Create virtual display with advanced features
    VirtualDisplayDesc desc;
    desc.name = "VDD Advanced Display";
    desc.preferredMode = { 3840, 2160, 60, 1 };
    desc.hdr10 = true;
    desc.stereoscopic = true;
    
    PrintStatus("Activate", Activate(desc, 1));
    
    if (IsActive()) {
        std::cout << "Advanced virtual display is active" << std::endl;
        
        // Configure HDR support
        PrintStatus("SetHdrSupport", SetHdrSupport(0, true));
        
        // Configure stereo support
        PrintStatus("SetStereoSupport", SetStereoSupport(0, true));
        
        // Set custom EDID
        std::vector<uint8_t> customEdid(128, 0);
        // TODO: Fill with actual EDID data
        PrintStatus("SetCustomEdid", SetCustomEdid(0, customEdid));
        
        // Get EDID
        std::vector<uint8_t> edidData;
        if (GetEdid(0, edidData) == Status::Ok) {
            std::cout << "EDID data size: " << edidData.size() << " bytes" << std::endl;
        }
        
        // Wait
        std::this_thread::sleep_for(std::chrono::seconds(3));
        
        // Deactivate
        PrintStatus("Deactivate", Deactivate());
    }
    
    // Shutdown
    PrintStatus("Shutdown", Shutdown());
}

void RecoveryExample() {
    std::cout << "\n=== Recovery Example ===" << std::endl;
    
    // Initialize SDK
    PrintStatus("Initialize", Initialize());
    
    // Check for orphaned state
    PrintStatus("RecoverOrphanedState", RecoverOrphanedState());
    
    // Ensure driver is running
    PrintStatus("EnsureDriverRunning", EnsureDriverRunning());
    
    // Shutdown
    PrintStatus("Shutdown", Shutdown());
}

int main() {
    std::cout << "VDD SDK Usage Examples" << std::endl;
    std::cout << "======================" << std::endl;
    
    try {
        // Run examples
        BasicUsageExample();
        VirtualDisplayExample();
        SessionManagementExample();
        AdvancedFeaturesExample();
        RecoveryExample();
        
        std::cout << "\nAll examples completed successfully!" << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
