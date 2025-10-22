/*++

Copyright (c) 2024 VDD SDK Project

Abstract:

    TDD tests for SetMode() function
    Tests virtual display mode setting functionality

Environment:

    User Mode, C++17

--*/

#include "vddsdk.h"
#include <iostream>
#include <string>
#include <vector>
#include <cassert>

using namespace vdd;

// Simple test framework
class SetModeTest {
public:
    static int totalTests;
    static int passedTests;
    static int failedTests;
    
    static void runTest(const std::string& testName, bool result) {
        totalTests++;
        if (result) {
            passedTests++;
            std::cout << "✅ " << testName << " - PASSED" << std::endl;
        } else {
            failedTests++;
            std::cout << "❌ " << testName << " - FAILED" << std::endl;
        }
    }
    
    static void printSummary() {
        std::cout << "\n========================================" << std::endl;
        std::cout << "SetMode() Test Summary:" << std::endl;
        std::cout << "Total Tests: " << totalTests << std::endl;
        std::cout << "Passed: " << passedTests << std::endl;
        std::cout << "Failed: " << failedTests << std::endl;
        std::cout << "Success Rate: " << (totalTests > 0 ? (passedTests * 100 / totalTests) : 0) << "%" << std::endl;
        std::cout << "========================================" << std::endl;
    }
};

// Static variable initialization
int SetModeTest::totalTests = 0;
int SetModeTest::passedTests = 0;
int SetModeTest::failedTests = 0;

// Test functions
void testSetModeBasic() {
    std::cout << "\nTesting basic SetMode() functionality..." << std::endl;
    
    // Initialize SDK
    SdkConfig config;
    config.enableLogging = true;
    config.maxLogLevel = 2; // Info level
    
    Status status = Initialize(config);
    SetModeTest::runTest("SDK Initialization", status == Status::Ok || status == Status::AlreadyInstalled);
    
    // 激活虛擬顯示器
    VirtualDisplayDesc desc;
    desc.name = "TestDisplay";
    desc.preferredMode = {1920, 1080, 60, 1};
    desc.hdr10 = false;
    desc.stereoscopic = false;
    
    status = Activate(desc, 1);
    SetModeTest::runTest("Activate Virtual Display", status == Status::Ok);
    
    // Set new display mode
    DisplayMode newMode = {2560, 1440, 75, 1};
    status = SetMode(0, newMode);
    SetModeTest::runTest("Set Display Mode", status == Status::Ok);
    
    // Verify mode setting
    DisplayMode currentMode;
    status = GetMode(0, currentMode);
    SetModeTest::runTest("獲取當前模式", status == Status::Ok);
    SetModeTest::runTest("模式寬度", currentMode.width == newMode.width);
    SetModeTest::runTest("模式高度", currentMode.height == newMode.height);
    SetModeTest::runTest("模式刷新率", currentMode.refreshNumerator == newMode.refreshNumerator);
}

void testSetModeInvalidIndex() {
    std::cout << "\nTesting SetMode() with invalid index..." << std::endl;
    
    // Initialize SDK
    SdkConfig config;
    config.enableLogging = true;
    config.maxLogLevel = 2; // Info level
    
    Status status = Initialize(config);
    SetModeTest::runTest("SDK Initialization", status == Status::Ok || status == Status::AlreadyInstalled);
    
    // 激活虛擬顯示器
    VirtualDisplayDesc desc;
    desc.name = "TestDisplay";
    desc.preferredMode = {1920, 1080, 60, 1};
    desc.hdr10 = false;
    desc.stereoscopic = false;
    
    status = Activate(desc, 1);
    SetModeTest::runTest("Activate Virtual Display", status == Status::Ok);
    
    // Try to set mode for invalid index
    DisplayMode newMode = {2560, 1440, 75, 1};
    status = SetMode(999, newMode); // 無效索引
    SetModeTest::runTest("Invalid Index Setting", status == Status::InvalidArg);
}

void testSetModeWithoutActivation() {
    std::cout << "\nTesting SetMode() without activation..." << std::endl;
    
    // Initialize SDK
    SdkConfig config;
    config.enableLogging = true;
    config.maxLogLevel = 2; // Info level
    
    Status status = Initialize(config);
    SetModeTest::runTest("SDK Initialization", status == Status::Ok || status == Status::AlreadyInstalled);
    
    // Try to set mode for unactivated display
    DisplayMode newMode = {2560, 1440, 75, 1};
    status = SetMode(0, newMode);
    SetModeTest::runTest("Unactivated State Setting", status == Status::NotActive);
}

void testSetModeInvalidResolution() {
    std::cout << "\nTesting SetMode() with invalid resolution..." << std::endl;
    
    // Initialize SDK
    SdkConfig config;
    config.enableLogging = true;
    config.maxLogLevel = 2; // Info level
    
    Status status = Initialize(config);
    SetModeTest::runTest("SDK Initialization", status == Status::Ok || status == Status::AlreadyInstalled);
    
    // 激活虛擬顯示器
    VirtualDisplayDesc desc;
    desc.name = "TestDisplay";
    desc.preferredMode = {1920, 1080, 60, 1};
    desc.hdr10 = false;
    desc.stereoscopic = false;
    
    status = Activate(desc, 1);
    SetModeTest::runTest("Activate Virtual Display", status == Status::Ok);
    
    // Try to set invalid resolution
    DisplayMode invalidMode = {0, 0, 60, 1}; // Invalid resolution
    status = SetMode(0, invalidMode);
    SetModeTest::runTest("Invalid Resolution Setting", status == Status::InvalidArg);
}

void testSetModeMultipleDisplays() {
    std::cout << "\nTesting SetMode() for multiple displays..." << std::endl;
    
    // Initialize SDK
    SdkConfig config;
    config.enableLogging = true;
    config.maxLogLevel = 2; // Info level
    
    Status status = Initialize(config);
    SetModeTest::runTest("SDK Initialization", status == Status::Ok || status == Status::AlreadyInstalled);
    
    // 激活多個虛擬顯示器
    VirtualDisplayDesc desc;
    desc.name = "MultiDisplay";
    desc.preferredMode = {1920, 1080, 60, 1};
    desc.hdr10 = false;
    desc.stereoscopic = false;
    
    status = Activate(desc, 3);
    SetModeTest::runTest("Activate Multiple Virtual Displays", status == Status::Ok);
    
    // Set different modes for each display
    DisplayMode mode1 = {1920, 1080, 60, 1};
    DisplayMode mode2 = {2560, 1440, 75, 1};
    DisplayMode mode3 = {3840, 2160, 30, 1};
    
    status = SetMode(0, mode1);
    SetModeTest::runTest("Set Display 0 Mode", status == Status::Ok);
    
    status = SetMode(1, mode2);
    SetModeTest::runTest("Set Display 1 Mode", status == Status::Ok);
    
    status = SetMode(2, mode3);
    SetModeTest::runTest("Set Display 2 Mode", status == Status::Ok);
    
    // Verify mode for each display
    DisplayMode currentMode;
    
    status = GetMode(0, currentMode);
    SetModeTest::runTest("獲取顯示器 0 模式", status == Status::Ok);
    SetModeTest::runTest("顯示器 0 模式寬度", currentMode.width == mode1.width);
    
    status = GetMode(1, currentMode);
    SetModeTest::runTest("獲取顯示器 1 模式", status == Status::Ok);
    SetModeTest::runTest("顯示器 1 模式寬度", currentMode.width == mode2.width);
    
    status = GetMode(2, currentMode);
    SetModeTest::runTest("獲取顯示器 2 模式", status == Status::Ok);
    SetModeTest::runTest("顯示器 2 模式寬度", currentMode.width == mode3.width);
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "SetMode() Function TDD Tests" << std::endl;
    std::cout << "========================================" << std::endl;
    
    try {
        // Run all tests
        testSetModeBasic();
        testSetModeInvalidIndex();
        testSetModeWithoutActivation();
        testSetModeInvalidResolution();
        testSetModeMultipleDisplays();
        
        // Print test summary
        SetModeTest::printSummary();
        
        return (SetModeTest::failedTests == 0) ? 0 : 1;
    }
    catch (const std::exception& e) {
        std::cout << "❌ Test execution exception: " << e.what() << std::endl;
        return 1;
    }
}
