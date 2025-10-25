/*++

Copyright (c) 2025 VDD SDK Project

Abstract:

    Performance testing tool for VDD SDK
    Measures actual execution times of Windows API calls

Environment:

    User Mode, C++17

--*/

#include "vddsdk.h"
#include <chrono>
#include <iostream>
#include <iomanip>

class PerformanceTimer {
public:
    void start() {
        start_time = std::chrono::high_resolution_clock::now();
    }
    
    void stop() {
        end_time = std::chrono::high_resolution_clock::now();
    }
    
    double get_milliseconds() {
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        return duration.count() / 1000.0;
    }
    
private:
    std::chrono::high_resolution_clock::time_point start_time;
    std::chrono::high_resolution_clock::time_point end_time;
};

void test_initialization_performance() {
    std::cout << "Testing Initialization Performance...\n";
    
    vdd::SdkConfig config;
    config.enableLogging = false;
    
    PerformanceTimer timer;
    timer.start();
    
    vdd::Status status = vdd::Initialize(config);
    
    timer.stop();
    
    std::cout << "Initialization: " << std::fixed << std::setprecision(2) 
              << timer.get_milliseconds() << "ms (Status: " 
              << (status == vdd::Status::Ok ? "Success" : "Failed") << ")\n";
}

void test_system_info_performance() {
    std::cout << "Testing System Info Performance...\n";
    
    PerformanceTimer timer;
    timer.start();
    
    std::string info = vdd::GetSystemInfo();
    
    timer.stop();
    
    std::cout << "System Info: " << std::fixed << std::setprecision(2) 
              << timer.get_milliseconds() << "ms (Size: " << info.length() << " chars)\n";
}

void test_adapter_enumeration_performance() {
    std::cout << "Testing Adapter Enumeration Performance...\n";
    
    std::vector<vdd::AdapterInfo> adapters;
    
    PerformanceTimer timer;
    timer.start();
    
    vdd::Status status = vdd::EnumerateAdapters(adapters);
    
    timer.stop();
    
    std::cout << "Adapter Enumeration: " << std::fixed << std::setprecision(2) 
              << timer.get_milliseconds() << "ms (Found: " << adapters.size() 
              << " adapters, Status: " << (status == vdd::Status::Ok ? "Success" : "Failed") << ")\n";
}

void test_driver_status_performance() {
    std::cout << "Testing Driver Status Performance...\n";
    
    PerformanceTimer timer;
    timer.start();
    
    bool installed = vdd::IsDriverInstalled();
    
    timer.stop();
    
    std::cout << "Driver Status Check: " << std::fixed << std::setprecision(2) 
              << timer.get_milliseconds() << "ms (Installed: " 
              << (installed ? "Yes" : "No") << ")\n";
}

void test_display_activation_performance() {
    std::cout << "Testing Display Activation Performance...\n";
    
    vdd::VirtualDisplayDesc desc;
    desc.name = "Performance Test Display";
    desc.preferredMode = {1920, 1080, 60, 1};
    desc.hdr10 = false;
    desc.stereoscopic = false;
    
    PerformanceTimer timer;
    timer.start();
    
    vdd::Status status = vdd::Activate(desc, 1);
    
    timer.stop();
    
    std::cout << "Display Activation: " << std::fixed << std::setprecision(2) 
              << timer.get_milliseconds() << "ms (Status: " 
              << (status == vdd::Status::Ok ? "Success" : "Failed") << ")\n";
}

void test_display_mode_performance() {
    std::cout << "Testing Display Mode Performance...\n";
    
    vdd::DisplayMode mode = {1920, 1080, 60, 1};
    
    PerformanceTimer timer;
    timer.start();
    
    vdd::Status status = vdd::SetMode(0, mode);
    
    timer.stop();
    
    std::cout << "Display Mode Change: " << std::fixed << std::setprecision(2) 
              << timer.get_milliseconds() << "ms (Status: " 
              << (status == vdd::Status::Ok ? "Success" : "Failed") << ")\n";
}

void test_display_location_performance() {
    std::cout << "Testing Display Location Performance...\n";
    
    vdd::DisplayRect rect = {100, 100, 2020, 1180};
    
    PerformanceTimer timer;
    timer.start();
    
    vdd::Status status = vdd::SetLocation(0, rect);
    
    timer.stop();
    
    std::cout << "Display Location Change: " << std::fixed << std::setprecision(2) 
              << timer.get_milliseconds() << "ms (Status: " 
              << (status == vdd::Status::Ok ? "Success" : "Failed") << ")\n";
}

int main() {
    std::cout << "VDD SDK Performance Test\n";
    std::cout << "=======================\n\n";
    
    // Test initialization
    test_initialization_performance();
    std::cout << "\n";
    
    // Test system info
    test_system_info_performance();
    std::cout << "\n";
    
    // Test adapter enumeration
    test_adapter_enumeration_performance();
    std::cout << "\n";
    
    // Test driver status
    test_driver_status_performance();
    std::cout << "\n";
    
    // Test display activation
    test_display_activation_performance();
    std::cout << "\n";
    
    // Test display mode
    test_display_mode_performance();
    std::cout << "\n";
    
    // Test display location
    test_display_location_performance();
    std::cout << "\n";
    
    // Cleanup
    vdd::Deactivate();
    vdd::Shutdown();
    
    std::cout << "Performance test completed.\n";
    return 0;
}
