#include "vddsdk.h"
#include <iostream>
#include <windows.h>

using namespace vdd;

void PrintStatus(const char* operation, Status status) {
    std::cout << operation << ": " << GetStatusString(status) << std::endl;
}

int main() {
    std::cout << "=== VDD SDK Test Program ===" << std::endl;
    
    // Test 1: Get version
    Version version = GetVersion();
    std::cout << "VDD SDK Version: " << version.major << "." << version.minor << "." << version.patch << std::endl;
    
    // Test 2: Test driver connection
    std::cout << "\n--- Testing Driver Connection ---" << std::endl;
    Status status = TestDriverConnection();
    PrintStatus("Driver Connection", status);
    
    if (status != Status::Ok) {
        std::cout << "Driver not available. Make sure the driver is installed and running." << std::endl;
        return 1;
    }
    
    // Test 3: Configure a virtual display
    std::cout << "\n--- Configuring Virtual Display ---" << std::endl;
    status = ConfigureDisplay(0, 1920, 1080, 60, "VDD Test Display");
    PrintStatus("Configure Display", status);
    
    // Test 4: Set monitor count
    std::cout << "\n--- Setting Monitor Count ---" << std::endl;
    status = SetMonitorCount(1);
    PrintStatus("Set Monitor Count", status);
    
    // Test 5: Test activation
    std::cout << "\n--- Testing Activation ---" << std::endl;
    VirtualDisplayDesc desc;
    desc.name = "VDD XR Display";
    desc.preferredMode.width = 1920;
    desc.preferredMode.height = 1080;
    desc.preferredMode.refreshNumerator = 90;
    desc.preferredMode.refreshDenominator = 1;
    desc.hdr10 = false;
    desc.stereoscopic = false;
    
    status = Activate(desc, 1);
    PrintStatus("Activate VDD", status);
    
    if (status == Status::Ok) {
        std::cout << "Virtual display activated successfully!" << std::endl;
        
        // Wait a bit to see the display
        std::cout << "Display should be visible for 5 seconds..." << std::endl;
        Sleep(5000);
        
        // Deactivate
        status = Deactivate();
        PrintStatus("Deactivate VDD", status);
    }
    
    std::cout << "\n=== Test Complete ===" << std::endl;
    return 0;
}

