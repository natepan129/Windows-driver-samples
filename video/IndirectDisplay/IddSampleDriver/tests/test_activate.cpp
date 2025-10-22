/*++

Copyright (c) 2025 VDD SDK Project

Abstract:

    TDD tests for Activate() function
    Tests virtual display activation functionality

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
class ActivateTest {
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
        std::cout << "Activate() Test Summary:" << std::endl;
        std::cout << "Total Tests: " << totalTests << std::endl;
        std::cout << "Passed: " << passedTests << std::endl;
        std::cout << "Failed: " << failedTests << std::endl;
        std::cout << "Success Rate: " << (totalTests > 0 ? (passedTests * 100 / totalTests) : 0) << "%" << std::endl;
        std::cout << "========================================" << std::endl;
    }
};

int ActivateTest::totalTests = 0;
int ActivateTest::passedTests = 0;
int ActivateTest::failedTests = 0;

// ============================================================================
// Activate() function tests
// ============================================================================

void testActivateBasic() {
    std::cout << "\nTesting basic Activate() functionality..." << std::endl;
    
    Initialize(SdkConfig{});
    
    VirtualDisplayDesc desc;
    desc.name = "Test Virtual Display";
    desc.preferredMode = { 1920, 1080, 60, 1 };
    desc.hdr10 = false;
    desc.stereoscopic = false;
    
    Status status = Activate(desc, 1);
    bool result = (status == Status::Ok);
    
    ActivateTest::runTest("Basic Activate() test", result);
    std::cout << "Activation status: " << StatusToString(status) << std::endl;
    
    if (result) {
        bool isActive = IsActive();
        uint32_t displayCount = GetActiveDisplayCount();
        std::cout << "Is active: " << (isActive ? "Yes" : "No") << std::endl;
        std::cout << "Display count: " << displayCount << std::endl;
    }
    
    Shutdown();
}

void testActivateMultipleDisplays() {
    std::cout << "\nTesting multiple display activation..." << std::endl;
    
    Initialize(SdkConfig{});
    
    VirtualDisplayDesc desc;
    desc.name = "Multi Display";
    desc.preferredMode = { 1920, 1080, 60, 1 };
    
    Status status = Activate(desc, 3); // Create 3 displays
    bool result = (status == Status::Ok);
    
    ActivateTest::runTest("Multiple display activation test", result);
    std::cout << "Activation status: " << StatusToString(status) << std::endl;
    
    if (result) {
        bool isActive = IsActive();
        uint32_t displayCount = GetActiveDisplayCount();
        std::cout << "Is active: " << (isActive ? "Yes" : "No") << std::endl;
        std::cout << "Display count: " << displayCount << std::endl;
    }
    
    Shutdown();
}

void testActivateWithHdr() {
    std::cout << "\ntest HDR displayactivation..." << std::endl;
    
    Initialize(SdkConfig{});
    
    VirtualDisplayDesc desc;
    desc.name = "HDR Display";
    desc.preferredMode = { 3840, 2160, 60, 1 };
    desc.hdr10 = true;
    desc.stereoscopic = false;
    
    Status status = Activate(desc, 1);
    bool result = (status == Status::Ok);
    
    ActivateTest::runTest("HDR displayactivationtest", result);
    std::cout << "Activation status: " << StatusToString(status) << std::endl;
    
    Shutdown();
}

void testActivateWithStereo() {
    std::cout << "\nTesting stereo display activation..." << std::endl;
    
    Initialize(SdkConfig{});
    
    VirtualDisplayDesc desc;
    desc.name = "Stereo Display";
    desc.preferredMode = { 1920, 1080, 90, 1 };
    desc.hdr10 = false;
    desc.stereoscopic = true;
    
    Status status = Activate(desc, 1);
    bool result = (status == Status::Ok);
    
    ActivateTest::runTest("Stereo display activation test", result);
    std::cout << "Activation status: " << StatusToString(status) << std::endl;
    
    Shutdown();
}

void testActivateInvalidParameters() {
    std::cout << "\nTesting invalid parameters..." << std::endl;
    
    Initialize(SdkConfig{});
    
    // Test empty name
    VirtualDisplayDesc desc1;
    desc1.name = ""; // Empty name
    desc1.preferredMode = { 1920, 1080, 60, 1 };
    
    Status status1 = Activate(desc1, 1);
    bool result1 = (status1 != Status::Ok);
    
    ActivateTest::runTest("Empty name test", result1);
    std::cout << "Empty name status: " << StatusToString(status1) << std::endl;
    
    // Test invalid mode
    VirtualDisplayDesc desc2;
    desc2.name = "Invalid Mode Display";
    desc2.preferredMode = { 0, 0, 0, 0 }; // Invalid mode
    
    Status status2 = Activate(desc2, 1);
    bool result2 = (status2 != Status::Ok);
    
    ActivateTest::runTest("Invalid mode test", result2);
    std::cout << "Invalid mode status: " << StatusToString(status2) << std::endl;
    
    // Test zero count
    VirtualDisplayDesc desc3;
    desc3.name = "Zero Count Display";
    desc3.preferredMode = { 1920, 1080, 60, 1 };
    
    Status status3 = Activate(desc3, 0); // Zero count
    bool result3 = (status3 != Status::Ok);
    
    ActivateTest::runTest("Zero count test", result3);
    std::cout << "Zero count status: " << StatusToString(status3) << std::endl;
    
    Shutdown();
}

void testActivateWithoutInitialize() {
    std::cout << "\nTesting activation without initialization..." << std::endl;
    
    // Don't call Initialize()
    VirtualDisplayDesc desc;
    desc.name = "Uninitialized Display";
    desc.preferredMode = { 1920, 1080, 60, 1 };
    
    Status status = Activate(desc, 1);
    bool result = (status == Status::NotInstalled);
    
    ActivateTest::runTest("Uninitialized Activation Test", result);
    std::cout << "Uninitialized status: " << StatusToString(status) << std::endl;
}

void testActivateAfterDeactivate() {
    std::cout << "\nTesting reactivation after deactivation..." << std::endl;
    
    Initialize(SdkConfig{});
    
    VirtualDisplayDesc desc;
    desc.name = "Reactivation Display";
    desc.preferredMode = { 1920, 1080, 60, 1 };
    
    // 第一次activation
    Status status1 = Activate(desc, 1);
    bool result1 = (status1 == Status::Ok);
    
    ActivateTest::runTest("第一次activationtest", result1);
    std::cout << "第一次activation狀態: " << StatusToString(status1) << std::endl;
    
    if (result1) {
        // 停用
        Status deactivateStatus = Deactivate();
        bool deactivateResult = (deactivateStatus == Status::Ok);
        
        ActivateTest::runTest("停用test", deactivateResult);
        std::cout << "停用狀態: " << StatusToString(deactivateStatus) << std::endl;
        
        if (deactivateResult) {
            // 重新activation
            Status status2 = Activate(desc, 1);
            bool result2 = (status2 == Status::Ok);
            
            ActivateTest::runTest("重新activationtest", result2);
            std::cout << "重新activation狀態: " << StatusToString(status2) << std::endl;
        }
    }
    
    Shutdown();
}

void testActivateStateManagement() {
    std::cout << "\ntestactivation狀態管理..." << std::endl;
    
    Initialize(SdkConfig{});
    
    // 初始狀態
    bool initialActive = IsActive();
    uint32_t initialCount = GetActiveDisplayCount();
    
    ActivateTest::runTest("初始未activation狀態", !initialActive);
    ActivateTest::runTest("初始display數量為零", initialCount == 0);
    
    // activation後狀態
    VirtualDisplayDesc desc;
    desc.name = "State Test Display";
    desc.preferredMode = { 1920, 1080, 60, 1 };
    
    Status status = Activate(desc, 1);
    if (status == Status::Ok) {
        bool afterActive = IsActive();
        uint32_t afterCount = GetActiveDisplayCount();
        
        ActivateTest::runTest("activation後狀態", afterActive);
        ActivateTest::runTest("activation後display數量", afterCount == 1);
        
        std::cout << "activation後狀態: " << (afterActive ? "activation" : "未activation") << std::endl;
        std::cout << "activation後數量: " << afterCount << std::endl;
    }
    
    Shutdown();
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "Activate() 函數 TDD test" << std::endl;
    std::cout << "========================================" << std::endl;
    
    try {
        // 運行所有test
        testActivateBasic();
        testActivateMultipleDisplays();
        testActivateWithHdr();
        testActivateWithStereo();
        testActivateInvalidParameters();
        testActivateWithoutInitialize();
        testActivateAfterDeactivate();
        testActivateStateManagement();
        
        // 打印test總結
        ActivateTest::printSummary();
        
        return (ActivateTest::failedTests == 0) ? 0 : 1;
    }
    catch (const std::exception& e) {
        std::cerr << "異常: " << e.what() << std::endl;
        return 1;
    }
}
