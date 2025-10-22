/*++

Copyright (c) 2024 VDD SDK Project

Abstract:

    Simplified TDD tests for Deactivate() function
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
    
    // Initialize SDK
    SdkConfig config;
    config.enableLogging = true;
    config.maxLogLevel = 2; // Info level
    
    Status status = Initialize(config);
    DeactivateTest::runTest("SDK Initialization", status == Status::Ok || status == Status::AlreadyInstalled);
    
    // Activate virtual display
    VirtualDisplayDesc desc;
    desc.name = "TestDisplay";
    desc.preferredMode = {1920, 1080, 60};
    desc.hdr10 = false;
    desc.stereoscopic = false;
    
    status = Activate(desc, 1);
    DeactivateTest::runTest("激活虛擬顯示器", status == Status::Ok);
    
    // 檢查激活狀態
    bool isActive = IsActive();
    DeactivateTest::runTest("激活後狀態檢查", isActive);
    
    uint32_t displayCount = GetActiveDisplayCount();
    DeactivateTest::runTest("激活後顯示器數量", displayCount == 1);
    
    // 停用虛擬顯示器
    status = Deactivate();
    DeactivateTest::runTest("停用虛擬顯示器", status == Status::Ok);
    
    // 檢查停用後狀態
    isActive = IsActive();
    DeactivateTest::runTest("停用後狀態檢查", !isActive);
    
    displayCount = GetActiveDisplayCount();
    DeactivateTest::runTest("停用後顯示器數量", displayCount == 0);
}

void testDeactivateWithoutActivation() {
    std::cout << "\n測試未激活狀態下停用..." << std::endl;
    
    // Initialize SDK
    SdkConfig config;
    config.enableLogging = true;
    config.maxLogLevel = 2; // Info level
    
    Status status = Initialize(config);
    DeactivateTest::runTest("SDK Initialization", status == Status::Ok || status == Status::AlreadyInstalled);
    
    // 檢查初始狀態
    bool isActive = IsActive();
    DeactivateTest::runTest("初始狀態檢查", !isActive);
    
    // 嘗試停用未激活的顯示器
    status = Deactivate();
    DeactivateTest::runTest("停用未激活顯示器", status == Status::NotActive);
    
    // 檢查狀態未改變
    isActive = IsActive();
    DeactivateTest::runTest("停用後狀態未改變", !isActive);
}

void testDeactivateWithoutInitialize() {
    std::cout << "\n測試未初始化狀態下停用..." << std::endl;
    
    // 嘗試停用未初始化的 SDK
    Status status = Deactivate();
    DeactivateTest::runTest("未初始化停用", status == Status::NotInstalled);
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "Deactivate() 函數 TDD 測試" << std::endl;
    std::cout << "========================================" << std::endl;
    
    try {
        // 運行所有測試
        testDeactivateBasic();
        testDeactivateWithoutActivation();
        testDeactivateWithoutInitialize();
        
        // 打印測試總結
        DeactivateTest::printSummary();
        
        return (DeactivateTest::failedTests == 0) ? 0 : 1;
    }
    catch (const std::exception& e) {
        std::cout << "❌ 測試執行異常: " << e.what() << std::endl;
        return 1;
    }
}
