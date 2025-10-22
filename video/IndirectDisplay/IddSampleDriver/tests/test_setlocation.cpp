/*++

Copyright (c) 2024 VDD SDK Project

Abstract:

    TDD tests for SetLocation() function
    Tests virtual display position setting functionality

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
class SetLocationTest {
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
        std::cout << "SetLocation() Test Summary:" << std::endl;
        std::cout << "Total Tests: " << totalTests << std::endl;
        std::cout << "Passed: " << passedTests << std::endl;
        std::cout << "Failed: " << failedTests << std::endl;
        std::cout << "Success Rate: " << (totalTests > 0 ? (passedTests * 100 / totalTests) : 0) << "%" << std::endl;
        std::cout << "========================================" << std::endl;
    }
};

// 靜態變量初始化
int SetLocationTest::totalTests = 0;
int SetLocationTest::passedTests = 0;
int SetLocationTest::failedTests = 0;

// 測試函數
void testSetLocationBasic() {
    std::cout << "\n測試基本 SetLocation() 功能..." << std::endl;
    
    // 初始化 SDK
    SdkConfig config;
    config.enableLogging = true;
    config.maxLogLevel = 2; // Info level
    
    Status status = Initialize(config);
    SetLocationTest::runTest("SDK 初始化", status == Status::Ok || status == Status::AlreadyInstalled);
    
    // 激活虛擬顯示器
    VirtualDisplayDesc desc;
    desc.name = "TestDisplay";
    desc.preferredMode = {1920, 1080, 60, 1};
    desc.hdr10 = false;
    desc.stereoscopic = false;
    
    status = Activate(desc, 1);
    SetLocationTest::runTest("激活虛擬顯示器", status == Status::Ok);
    
    // 設置顯示器位置
    DisplayRect newLocation = {100, 200, 1920, 1080};
    status = SetLocation(0, newLocation);
    SetLocationTest::runTest("設置顯示器位置", status == Status::Ok);
    
    // 驗證位置設置
    DisplayRect currentLocation;
    status = GetLocation(0, currentLocation);
    SetLocationTest::runTest("獲取當前位置", status == Status::Ok);
    SetLocationTest::runTest("位置 X 坐標", currentLocation.x == newLocation.x);
    SetLocationTest::runTest("位置 Y 坐標", currentLocation.y == newLocation.y);
    SetLocationTest::runTest("位置寬度", currentLocation.width == newLocation.width);
    SetLocationTest::runTest("位置高度", currentLocation.height == newLocation.height);
}

void testSetLocationInvalidIndex() {
    std::cout << "\n測試無效索引 SetLocation()..." << std::endl;
    
    // 初始化 SDK
    SdkConfig config;
    config.enableLogging = true;
    config.maxLogLevel = 2; // Info level
    
    Status status = Initialize(config);
    SetLocationTest::runTest("SDK 初始化", status == Status::Ok || status == Status::AlreadyInstalled);
    
    // 激活虛擬顯示器
    VirtualDisplayDesc desc;
    desc.name = "TestDisplay";
    desc.preferredMode = {1920, 1080, 60, 1};
    desc.hdr10 = false;
    desc.stereoscopic = false;
    
    status = Activate(desc, 1);
    SetLocationTest::runTest("激活虛擬顯示器", status == Status::Ok);
    
    // 嘗試設置無效索引的位置
    DisplayRect newLocation = {100, 200, 1920, 1080};
    status = SetLocation(999, newLocation); // 無效索引
    SetLocationTest::runTest("無效索引設置", status == Status::InvalidArg);
}

void testSetLocationWithoutActivation() {
    std::cout << "\n測試未激活狀態下 SetLocation()..." << std::endl;
    
    // 初始化 SDK
    SdkConfig config;
    config.enableLogging = true;
    config.maxLogLevel = 2; // Info level
    
    Status status = Initialize(config);
    SetLocationTest::runTest("SDK 初始化", status == Status::Ok || status == Status::AlreadyInstalled);
    
    // 嘗試設置未激活顯示器的位置
    DisplayRect newLocation = {100, 200, 1920, 1080};
    status = SetLocation(0, newLocation);
    SetLocationTest::runTest("未激活狀態設置", status == Status::NotActive);
}

void testSetLocationInvalidRect() {
    std::cout << "\n測試無效矩形 SetLocation()..." << std::endl;
    
    // 初始化 SDK
    SdkConfig config;
    config.enableLogging = true;
    config.maxLogLevel = 2; // Info level
    
    Status status = Initialize(config);
    SetLocationTest::runTest("SDK 初始化", status == Status::Ok || status == Status::AlreadyInstalled);
    
    // 激活虛擬顯示器
    VirtualDisplayDesc desc;
    desc.name = "TestDisplay";
    desc.preferredMode = {1920, 1080, 60, 1};
    desc.hdr10 = false;
    desc.stereoscopic = false;
    
    status = Activate(desc, 1);
    SetLocationTest::runTest("激活虛擬顯示器", status == Status::Ok);
    
    // 嘗試設置無效矩形
    DisplayRect invalidRect = {0, 0, 0, 0}; // 無效矩形
    status = SetLocation(0, invalidRect);
    SetLocationTest::runTest("無效矩形設置", status == Status::InvalidArg);
}

void testSetLocationMultipleDisplays() {
    std::cout << "\n測試多個顯示器 SetLocation()..." << std::endl;
    
    // 初始化 SDK
    SdkConfig config;
    config.enableLogging = true;
    config.maxLogLevel = 2; // Info level
    
    Status status = Initialize(config);
    SetLocationTest::runTest("SDK 初始化", status == Status::Ok || status == Status::AlreadyInstalled);
    
    // 激活多個虛擬顯示器
    VirtualDisplayDesc desc;
    desc.name = "MultiDisplay";
    desc.preferredMode = {1920, 1080, 60, 1};
    desc.hdr10 = false;
    desc.stereoscopic = false;
    
    status = Activate(desc, 3);
    SetLocationTest::runTest("激活多個虛擬顯示器", status == Status::Ok);
    
    // 為每個顯示器設置不同的位置
    DisplayRect location1 = {0, 0, 1920, 1080};
    DisplayRect location2 = {1920, 0, 1920, 1080};
    DisplayRect location3 = {0, 1080, 1920, 1080};
    
    status = SetLocation(0, location1);
    SetLocationTest::runTest("設置顯示器 0 位置", status == Status::Ok);
    
    status = SetLocation(1, location2);
    SetLocationTest::runTest("設置顯示器 1 位置", status == Status::Ok);
    
    status = SetLocation(2, location3);
    SetLocationTest::runTest("設置顯示器 2 位置", status == Status::Ok);
    
    // 驗證每個顯示器的位置
    DisplayRect currentLocation;
    
    status = GetLocation(0, currentLocation);
    SetLocationTest::runTest("獲取顯示器 0 位置", status == Status::Ok);
    SetLocationTest::runTest("顯示器 0 X 坐標", currentLocation.x == location1.x);
    
    status = GetLocation(1, currentLocation);
    SetLocationTest::runTest("獲取顯示器 1 位置", status == Status::Ok);
    SetLocationTest::runTest("顯示器 1 X 坐標", currentLocation.x == location2.x);
    
    status = GetLocation(2, currentLocation);
    SetLocationTest::runTest("獲取顯示器 2 位置", status == Status::Ok);
    SetLocationTest::runTest("顯示器 2 Y 坐標", currentLocation.y == location3.y);
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "SetLocation() 函數 TDD 測試" << std::endl;
    std::cout << "========================================" << std::endl;
    
    try {
        // 運行所有測試
        testSetLocationBasic();
        testSetLocationInvalidIndex();
        testSetLocationWithoutActivation();
        testSetLocationInvalidRect();
        testSetLocationMultipleDisplays();
        
        // 打印測試總結
        SetLocationTest::printSummary();
        
        return (SetLocationTest::failedTests == 0) ? 0 : 1;
    }
    catch (const std::exception& e) {
        std::cout << "❌ 測試執行異常: " << e.what() << std::endl;
        return 1;
    }
}
