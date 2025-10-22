/*++

Copyright (c) 2024 VDD SDK Project

Abstract:

    TDD tests for Deactivate() function
    Tests virtual display deactivation functionality

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
class DeactivateTest {
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
        std::cout << "Deactivate() Test Summary:" << std::endl;
        std::cout << "Total Tests: " << totalTests << std::endl;
        std::cout << "Passed: " << passedTests << std::endl;
        std::cout << "Failed: " << failedTests << std::endl;
        std::cout << "Success Rate: " << (totalTests > 0 ? (passedTests * 100 / totalTests) : 0) << "%" << std::endl;
        std::cout << "========================================" << std::endl;
    }
};

// Static variable initialization
int DeactivateTest::totalTests = 0;
int DeactivateTest::passedTests = 0;
int DeactivateTest::failedTests = 0;

// Test functions
void testDeactivateBasic() {
    std::cout << "\nTesting basic Deactivate() functionality..." << std::endl;
    
    // Use global API functions, no class instance needed
    SdkConfig config;
    config.enableLogging = true;
    config.maxLogLevel = 2; // Info level
    
    // Initialize SDK
    Status status = sdk.Initialize(config);
    DeactivateTest::runTest("SDK Initialization", status == Status::Ok);
    
    // 激活虛擬顯示器
    VirtualDisplayDesc desc;
    desc.name = "TestDisplay";
    desc.preferredMode = {1920, 1080, 60};
    desc.hdr10 = false;
    desc.stereoscopic = false;
    
    status = sdk.Activate(desc, 1);
    DeactivateTest::runTest("Activate Virtual Display", status == Status::Ok);
    
    // Check activation status
    bool isActive = sdk.IsActive();
    DeactivateTest::runTest("Post-Activation Status Check", isActive);
    
    uint32_t displayCount = sdk.GetActiveDisplayCount();
    DeactivateTest::runTest("激活後顯示器數量", displayCount == 1);
    
    // Deactivate virtual display
    status = sdk.Deactivate();
    DeactivateTest::runTest("Deactivate Virtual Display", status == Status::Ok);
    
    // Check post-deactivation status
    isActive = sdk.IsActive();
    DeactivateTest::runTest("Post-Deactivation Status Check", !isActive);
    
    displayCount = sdk.GetActiveDisplayCount();
    DeactivateTest::runTest("Post-Deactivation Display Count", displayCount == 0);
}

void testDeactivateMultipleDisplays() {
    std::cout << "\nTesting multiple display deactivation..." << std::endl;
    
    // Use global API functions, no class instance needed
    SdkConfig config;
    config.enableLogging = true;
    config.maxLogLevel = 2; // Info level
    
    // Initialize SDK
    Status status = sdk.Initialize(config);
    DeactivateTest::runTest("SDK Initialization", status == Status::Ok);
    
    // 激活多個虛擬顯示器
    VirtualDisplayDesc desc;
    desc.name = "MultiDisplay";
    desc.preferredMode = {1920, 1080, 60};
    desc.hdr10 = false;
    desc.stereoscopic = false;
    
    status = sdk.Activate(desc, 3);
    DeactivateTest::runTest("Activate Multiple Virtual Displays", status == Status::Ok);
    
    // Check activation status
    bool isActive = sdk.IsActive();
    DeactivateTest::runTest("Post-Activation Status Check", isActive);
    
    uint32_t displayCount = sdk.GetActiveDisplayCount();
    DeactivateTest::runTest("激活後顯示器數量", displayCount == 3);
    
    // 停用所有虛擬顯示器
    status = sdk.Deactivate();
    DeactivateTest::runTest("Deactivate All Virtual Displays", status == Status::Ok);
    
    // Check post-deactivation status
    isActive = sdk.IsActive();
    DeactivateTest::runTest("Post-Deactivation Status Check", !isActive);
    
    displayCount = sdk.GetActiveDisplayCount();
    DeactivateTest::runTest("Post-Deactivation Display Count", displayCount == 0);
}

void testDeactivateWithoutActivation() {
    std::cout << "\nTesting deactivation without activation..." << std::endl;
    
    // Use global API functions, no class instance needed
    SdkConfig config;
    config.enableLogging = true;
    config.maxLogLevel = 2; // Info level
    
    // Initialize SDK
    Status status = sdk.Initialize(config);
    DeactivateTest::runTest("SDK Initialization", status == Status::Ok);
    
    // Check initial status
    bool isActive = sdk.IsActive();
    DeactivateTest::runTest("Initial Status Check", !isActive);
    
    // Try to deactivate unactivated display
    status = sdk.Deactivate();
    DeactivateTest::runTest("Deactivate Unactivated Display", status == Status::NotActive);
    
    // Check status unchanged
    isActive = sdk.IsActive();
    DeactivateTest::runTest("Status Unchanged After Deactivation", !isActive);
}

void testDeactivateWithoutInitialize() {
    std::cout << "\nTesting deactivation without initialization..." << std::endl;
    
    // Use global API functions, no class instance needed
    
    // Try to deactivate uninitialized SDK
    Status status = sdk.Deactivate();
    DeactivateTest::runTest("Deactivate Uninitialized SDK", status == Status::NotInstalled);
}

void testDeactivateStateManagement() {
    std::cout << "\nTesting deactivation state management..." << std::endl;
    
    // Use global API functions, no class instance needed
    SdkConfig config;
    config.enableLogging = true;
    config.maxLogLevel = 2; // Info level
    
    // Initialize SDK
    Status status = sdk.Initialize(config);
    DeactivateTest::runTest("SDK Initialization", status == Status::Ok);
    
    // 激活虛擬顯示器
    VirtualDisplayDesc desc;
    desc.name = "StateTestDisplay";
    desc.preferredMode = {1920, 1080, 60};
    desc.hdr10 = false;
    desc.stereoscopic = false;
    
    status = sdk.Activate(desc, 1);
    DeactivateTest::runTest("Activate Virtual Display", status == Status::Ok);
    
    // Check activation status
    bool isActive = sdk.IsActive();
    DeactivateTest::runTest("Activation Status Check", isActive);
    
    uint32_t displayCount = sdk.GetActiveDisplayCount();
    DeactivateTest::runTest("激活顯示器數量", displayCount == 1);
    
    // Deactivate virtual display
    status = sdk.Deactivate();
    DeactivateTest::runTest("Deactivate Virtual Display", status == Status::Ok);
    
    // Check post-deactivation status
    isActive = sdk.IsActive();
    DeactivateTest::runTest("Post-Deactivation Status Check", !isActive);
    
    displayCount = sdk.GetActiveDisplayCount();
    DeactivateTest::runTest("Post-Deactivation Display Count", displayCount == 0);
    
    // Try to deactivate again
    status = sdk.Deactivate();
    DeactivateTest::runTest("Repeat Deactivation", status == Status::NotActive);
    
    // Check status unchanged
    isActive = sdk.IsActive();
    DeactivateTest::runTest("Status After Repeat Deactivation", !isActive);
}

void testDeactivateErrorHandling() {
    std::cout << "\nTesting deactivation error handling..." << std::endl;
    
    // Use global API functions, no class instance needed
    SdkConfig config;
    config.enableLogging = true;
    config.maxLogLevel = 2; // Info level
    
    // Initialize SDK
    Status status = sdk.Initialize(config);
    DeactivateTest::runTest("SDK Initialization", status == Status::Ok);
    
    // 激活虛擬顯示器
    VirtualDisplayDesc desc;
    desc.name = "ErrorTestDisplay";
    desc.preferredMode = {1920, 1080, 60};
    desc.hdr10 = false;
    desc.stereoscopic = false;
    
    status = sdk.Activate(desc, 1);
    DeactivateTest::runTest("Activate Virtual Display", status == Status::Ok);
    
    // Deactivate virtual display
    status = sdk.Deactivate();
    DeactivateTest::runTest("Deactivate Virtual Display", status == Status::Ok);
    
    // Check error message
    std::string errorMsg = sdk.GetLastError();
    DeactivateTest::runTest("Error Message Check", !errorMsg.empty());
    
    // Check post-deactivation status
    bool isActive = sdk.IsActive();
    DeactivateTest::runTest("Post-Deactivation Status Check", !isActive);
    
    uint32_t displayCount = sdk.GetActiveDisplayCount();
    DeactivateTest::runTest("Post-Deactivation Display Count", displayCount == 0);
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "Deactivate() Function TDD Tests" << std::endl;
    std::cout << "========================================" << std::endl;
    
    try {
        // Run all tests
        testDeactivateBasic();
        testDeactivateMultipleDisplays();
        testDeactivateWithoutActivation();
        testDeactivateWithoutInitialize();
        testDeactivateStateManagement();
        testDeactivateErrorHandling();
        
        // Print test summary
        DeactivateTest::printSummary();
        
        return (DeactivateTest::failedTests == 0) ? 0 : 1;
    }
    catch (const std::exception& e) {
        std::cout << "❌ Test execution exception: " << e.what() << std::endl;
        return 1;
    }
}
