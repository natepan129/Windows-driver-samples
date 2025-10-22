/*++

Copyright (c) 2024 VDD SDK Project

Abstract:

    TDD tests for SetPrimary() function
    Tests virtual display primary display setting functionality

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
class SetPrimaryTest {
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
        std::cout << "SetPrimary() Test Summary:" << std::endl;
        std::cout << "Total Tests: " << totalTests << std::endl;
        std::cout << "Passed: " << passedTests << std::endl;
        std::cout << "Failed: " << failedTests << std::endl;
        std::cout << "Success Rate: " << (totalTests > 0 ? (passedTests * 100 / totalTests) : 0) << "%" << std::endl;
        std::cout << "========================================" << std::endl;
    }
};

// Static variable initialization
int SetPrimaryTest::totalTests = 0;
int SetPrimaryTest::passedTests = 0;
int SetPrimaryTest::failedTests = 0;

// Test functions
void testSetPrimaryBasic() {
    std::cout << "\nTesting basic SetPrimary() functionality..." << std::endl;
    
    // Initialize SDK
    SdkConfig config;
    config.enableLogging = true;
    config.maxLogLevel = 2; // Info level
    
    Status status = Initialize(config);
    SetPrimaryTest::runTest("SDK Initialization", status == Status::Ok || status == Status::AlreadyInstalled);
    
    // 激活虛擬顯示器
    VirtualDisplayDesc desc;
    desc.name = "TestDisplay";
    desc.preferredMode = {1920, 1080, 60, 1};
    desc.hdr10 = false;
    desc.stereoscopic = false;
    
    status = Activate(desc, 1);
    SetPrimaryTest::runTest("Activate Virtual Display", status == Status::Ok);
    
    // Set as primary display
    status = SetPrimary(0);
    SetPrimaryTest::runTest("Set Primary Display", status == Status::Ok);
}

void testSetPrimaryInvalidIndex() {
    std::cout << "\nTesting SetPrimary() with invalid index..." << std::endl;
    
    // Initialize SDK
    SdkConfig config;
    config.enableLogging = true;
    config.maxLogLevel = 2; // Info level
    
    Status status = Initialize(config);
    SetPrimaryTest::runTest("SDK Initialization", status == Status::Ok || status == Status::AlreadyInstalled);
    
    // 激活虛擬顯示器
    VirtualDisplayDesc desc;
    desc.name = "TestDisplay";
    desc.preferredMode = {1920, 1080, 60, 1};
    desc.hdr10 = false;
    desc.stereoscopic = false;
    
    status = Activate(desc, 1);
    SetPrimaryTest::runTest("Activate Virtual Display", status == Status::Ok);
    
    // Try to set invalid index as primary display
    status = SetPrimary(999); // 無效索引
    SetPrimaryTest::runTest("Invalid Index Setting", status == Status::InvalidArg);
}

void testSetPrimaryWithoutActivation() {
    std::cout << "\nTesting SetPrimary() without activation..." << std::endl;
    
    // Initialize SDK
    SdkConfig config;
    config.enableLogging = true;
    config.maxLogLevel = 2; // Info level
    
    Status status = Initialize(config);
    SetPrimaryTest::runTest("SDK Initialization", status == Status::Ok || status == Status::AlreadyInstalled);
    
    // Try to set unactivated display as primary
    status = SetPrimary(0);
    SetPrimaryTest::runTest("Unactivated State Setting", status == Status::NotActive);
}

void testSetPrimaryMultipleDisplays() {
    std::cout << "\nTesting SetPrimary() for multiple displays..." << std::endl;
    
    // Initialize SDK
    SdkConfig config;
    config.enableLogging = true;
    config.maxLogLevel = 2; // Info level
    
    Status status = Initialize(config);
    SetPrimaryTest::runTest("SDK Initialization", status == Status::Ok || status == Status::AlreadyInstalled);
    
    // 激活多個虛擬顯示器
    VirtualDisplayDesc desc;
    desc.name = "MultiDisplay";
    desc.preferredMode = {1920, 1080, 60, 1};
    desc.hdr10 = false;
    desc.stereoscopic = false;
    
    status = Activate(desc, 3);
    SetPrimaryTest::runTest("Activate Multiple Virtual Displays", status == Status::Ok);
    
    // Set first display as primary
    status = SetPrimary(0);
    SetPrimaryTest::runTest("Set Display 0 as Primary", status == Status::Ok);
    
    // Switch to second display as primary
    status = SetPrimary(1);
    SetPrimaryTest::runTest("切換到顯示器 1 為主顯示器", status == Status::Ok);
    
    // 切換到第三個顯示器為主顯示器
    status = SetPrimary(2);
    SetPrimaryTest::runTest("Switch to Display 2 as Primary", status == Status::Ok);
}

void testSetPrimarySwitching() {
    std::cout << "\nTesting primary display switching..." << std::endl;
    
    // Initialize SDK
    SdkConfig config;
    config.enableLogging = true;
    config.maxLogLevel = 2; // Info level
    
    Status status = Initialize(config);
    SetPrimaryTest::runTest("SDK Initialization", status == Status::Ok || status == Status::AlreadyInstalled);
    
    // 激活多個虛擬顯示器
    VirtualDisplayDesc desc;
    desc.name = "SwitchDisplay";
    desc.preferredMode = {1920, 1080, 60, 1};
    desc.hdr10 = false;
    desc.stereoscopic = false;
    
    status = Activate(desc, 2);
    SetPrimaryTest::runTest("Activate Multiple Virtual Displays", status == Status::Ok);
    
    // Set first display as primary
    status = SetPrimary(0);
    SetPrimaryTest::runTest("Set Display 0 as Primary", status == Status::Ok);
    
    // Switch to second display as primary
    status = SetPrimary(1);
    SetPrimaryTest::runTest("切換到顯示器 1 為主顯示器", status == Status::Ok);
    
    // 再次切換回第一個顯示器
    status = SetPrimary(0);
    SetPrimaryTest::runTest("Switch Back to Display 0 as Primary", status == Status::Ok);
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "SetPrimary() Function TDD Tests" << std::endl;
    std::cout << "========================================" << std::endl;
    
    try {
        // Run all tests
        testSetPrimaryBasic();
        testSetPrimaryInvalidIndex();
        testSetPrimaryWithoutActivation();
        testSetPrimaryMultipleDisplays();
        testSetPrimarySwitching();
        
        // Print test summary
        SetPrimaryTest::printSummary();
        
        return (SetPrimaryTest::failedTests == 0) ? 0 : 1;
    }
    catch (const std::exception& e) {
        std::cout << "❌ Test execution exception: " << e.what() << std::endl;
        std::cout << "Exception details: " << e.what() << std::endl;
        return 1;
    }
}
