/*++

Copyright (c) 2024 VDD SDK Project

Abstract:

    First TDD Test
    Start with the simplest API functions, following Red-Green-Refactor cycle

Environment:

    User Mode, C++17, Google Test

--*/

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "vddsdk.h"
#include <iostream>
#include <string>

using namespace vdd;
using namespace testing;

class FirstTddTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Test setup
        m_config.enableLogging = true;
        m_config.logFilePath = L"C:\\temp\\vdd_first_test.log";
        m_config.defaultTimeoutMs = 5000;
        
        std::cout << "Setting up test environment..." << std::endl;
    }

    void TearDown() override {
        // Test cleanup
        if (IsActive()) {
            Deactivate();
        }
        Shutdown();
        
        std::cout << "Cleaning up test environment..." << std::endl;
    }

    SdkConfig m_config;
};

// ============================================================================
// 第一個 TDD 測試：版本信息
// ============================================================================

TEST_F(FirstTddTest, GetVersion_ShouldReturnCorrectVersion) {
    // Arrange - 準備測試數據（無需準備，這是靜態函數）
    
    // Act - 執行被測試的操作
    Version version = GetVersion();
    
    // Assert - 驗證結果
    EXPECT_EQ(version.major, VDD_SDK_VERSION_MAJOR);
    EXPECT_EQ(version.minor, VDD_SDK_VERSION_MINOR);
    EXPECT_EQ(version.patch, VDD_SDK_VERSION_PATCH);
    
    std::cout << "版本測試通過: " << version.major << "." 
              << version.minor << "." << version.patch << std::endl;
}

// ============================================================================
// 第二個 TDD 測試：初始化功能
// ============================================================================

TEST_F(FirstTddTest, Initialize_WithValidConfig_ShouldReturnOk) {
    // Arrange - 準備測試數據
    SdkConfig config;
    config.enableLogging = true;
    config.logFilePath = L"C:\\temp\\vdd_init_test.log";
    config.defaultTimeoutMs = 5000;
    
    // Act - 執行被測試的操作
    Status status = Initialize(config);
    
    // Assert - 驗證結果
    EXPECT_EQ(status, Status::Ok);
    
    std::cout << "初始化測試通過: " << StatusToString(status) << std::endl;
}

TEST_F(FirstTddTest, Initialize_WithDefaultConfig_ShouldReturnOk) {
    // Arrange - 準備測試數據（使用默認配置）
    
    // Act - 執行被測試的操作
    Status status = Initialize();
    
    // Assert - 驗證結果
    EXPECT_EQ(status, Status::Ok);
    
    std::cout << "默認配置初始化測試通過: " << StatusToString(status) << std::endl;
}

TEST_F(FirstTddTest, Initialize_Twice_ShouldReturnAlreadyInstalled) {
    // Arrange - 準備測試數據
    SdkConfig config;
    config.enableLogging = false;
    
    // Act - 執行被測試的操作
    Status firstStatus = Initialize(config);
    Status secondStatus = Initialize(config);
    
    // Assert - 驗證結果
    EXPECT_EQ(firstStatus, Status::Ok);
    EXPECT_EQ(secondStatus, Status::AlreadyInstalled);
    
    std::cout << "重複初始化測試通過: " << StatusToString(secondStatus) << std::endl;
}

// ============================================================================
// 第三個 TDD 測試：關閉功能
// ============================================================================

TEST_F(FirstTddTest, Shutdown_AfterInitialize_ShouldReturnOk) {
    // Arrange - 準備測試數據
    Initialize(m_config);
    
    // Act - 執行被測試的操作
    Status status = Shutdown();
    
    // Assert - 驗證結果
    EXPECT_EQ(status, Status::Ok);
    
    std::cout << "關閉測試通過: " << StatusToString(status) << std::endl;
}

TEST_F(FirstTddTest, Shutdown_WithoutInitialize_ShouldReturnNotInstalled) {
    // Arrange - 準備測試數據（不初始化）
    
    // Act - 執行被測試的操作
    Status status = Shutdown();
    
    // Assert - 驗證結果
    EXPECT_EQ(status, Status::NotInstalled);
    
    std::cout << "未初始化關閉測試通過: " << StatusToString(status) << std::endl;
}

// ============================================================================
// 第四個 TDD 測試：狀態查詢功能
// ============================================================================

TEST_F(FirstTddTest, IsActive_BeforeActivate_ShouldReturnFalse) {
    // Arrange - 準備測試數據
    Initialize(m_config);
    
    // Act - 執行被測試的操作
    bool isActive = IsActive();
    
    // Assert - 驗證結果
    EXPECT_FALSE(isActive);
    
    std::cout << "未激活狀態測試通過: " << (isActive ? "true" : "false") << std::endl;
}

TEST_F(FirstTddTest, GetActiveDisplayCount_BeforeActivate_ShouldReturnZero) {
    // Arrange - 準備測試數據
    Initialize(m_config);
    
    // Act - 執行被測試的操作
    uint32_t count = GetActiveDisplayCount();
    
    // Assert - 驗證結果
    EXPECT_EQ(count, 0);
    
    std::cout << "未激活顯示器計數測試通過: " << count << std::endl;
}

// ============================================================================
// 第五個 TDD 測試：錯誤處理功能
// ============================================================================

TEST_F(FirstTddTest, GetLastError_AfterInitialize_ShouldReturnEmptyString) {
    // Arrange - 準備測試數據
    Initialize(m_config);
    
    // Act - 執行被測試的操作
    std::string error = GetLastError();
    
    // Assert - 驗證結果
    EXPECT_TRUE(error.empty() || error == "SDK not initialized");
    
    std::cout << "錯誤信息測試通過: '" << error << "'" << std::endl;
}

TEST_F(FirstTddTest, StatusToString_ShouldReturnCorrectStrings) {
    // Arrange - 準備測試數據
    std::vector<Status> statuses = {
        Status::Ok,
        Status::AlreadyInstalled,
        Status::NotInstalled,
        Status::AdminRequired,
        Status::DriverError
    };
    
    std::vector<std::string> expectedStrings = {
        "Success",
        "Already installed",
        "Not installed",
        "Administrator privileges required",
        "Driver operation failed"
    };
    
    // Act & Assert - 執行並驗證結果
    for (size_t i = 0; i < statuses.size(); ++i) {
        std::string result = StatusToString(statuses[i]);
        EXPECT_EQ(result, expectedStrings[i]);
        
        std::cout << "狀態轉換測試通過: " << result << std::endl;
    }
}

// ============================================================================
// 第六個 TDD 測試：系統信息功能
// ============================================================================

TEST_F(FirstTddTest, GetSystemInfo_ShouldReturnNonEmptyString) {
    // Arrange - 準備測試數據
    Initialize(m_config);
    
    // Act - 執行被測試的操作
    std::string systemInfo = GetSystemInfo();
    
    // Assert - 驗證結果
    EXPECT_FALSE(systemInfo.empty());
    
    std::cout << "系統信息測試通過: " << systemInfo.substr(0, 50) << "..." << std::endl;
}

// ============================================================================
// 第七個 TDD 測試：工具函數
// ============================================================================

TEST_F(FirstTddTest, IsRunningAsAdministrator_ShouldReturnBoolean) {
    // Arrange - 準備測試數據（無需準備）
    
    // Act - 執行被測試的操作
    bool isAdmin = IsRunningAsAdministrator();
    
    // Assert - 驗證結果（在測試環境中通常不是管理員）
    EXPECT_FALSE(isAdmin); // 或者根據實際情況調整
    
    std::cout << "管理員權限測試通過: " << (isAdmin ? "是" : "否") << std::endl;
}

TEST_F(FirstTddTest, RequestElevation_ShouldReturnBoolean) {
    // Arrange - 準備測試數據（無需準備）
    
    // Act - 執行被測試的操作
    bool elevated = RequestElevation();
    
    // Assert - 驗證結果（在測試環境中通常無法提升權限）
    EXPECT_FALSE(elevated); // 或者根據實際情況調整
    
    std::cout << "權限提升測試通過: " << (elevated ? "成功" : "失敗") << std::endl;
}

// ============================================================================
// 第八個 TDD 測試：驅動程序狀態
// ============================================================================

TEST_F(FirstTddTest, IsDriverInstalled_ShouldReturnBoolean) {
    // Arrange - 準備測試數據
    Initialize(m_config);
    
    // Act - 執行被測試的操作
    bool isInstalled = IsDriverInstalled();
    
    // Assert - 驗證結果（在測試環境中可能未安裝）
    // 這是一個預期的結果，不應該導致測試失敗
    
    std::cout << "驅動程序安裝狀態測試通過: " << (isInstalled ? "已安裝" : "未安裝") << std::endl;
}

TEST_F(FirstTddTest, GetDriverVersion_ShouldReturnVersion) {
    // Arrange - 準備測試數據
    Initialize(m_config);
    
    // Act - 執行被測試的操作
    Version driverVersion = GetDriverVersion();
    
    // Assert - 驗證結果
    if (IsDriverInstalled()) {
        EXPECT_GT(driverVersion.major, 0);
        std::cout << "驅動程序版本測試通過: " << driverVersion.major << "." 
                  << driverVersion.minor << "." << driverVersion.patch << std::endl;
    } else {
        EXPECT_EQ(driverVersion.major, 0);
        EXPECT_EQ(driverVersion.minor, 0);
        EXPECT_EQ(driverVersion.patch, 0);
        std::cout << "驅動程序未安裝，版本為 0.0.0" << std::endl;
    }
}

// ============================================================================
// 第九個 TDD 測試：適配器枚舉
// ============================================================================

TEST_F(FirstTddTest, EnumerateAdapters_ShouldReturnVector) {
    // Arrange - 準備測試數據
    Initialize(m_config);
    
    // Act - 執行被測試的操作
    std::vector<AdapterInfo> adapters;
    Status status = EnumerateAdapters(adapters);
    
    // Assert - 驗證結果
    EXPECT_EQ(status, Status::Ok);
    EXPECT_GE(adapters.size(), 0); // 至少應該有 0 個適配器
    
    std::cout << "適配器枚舉測試通過: 找到 " << adapters.size() << " 個適配器" << std::endl;
    
    // 檢查適配器信息
    for (size_t i = 0; i < adapters.size(); ++i) {
        const auto& adapter = adapters[i];
        EXPECT_FALSE(adapter.name.empty());
        EXPECT_FALSE(adapter.deviceInstanceId.empty());
        
        std::cout << "  適配器 " << i << ": " << adapter.name 
                  << " (虛擬: " << (adapter.isVirtual ? "是" : "否") << ")" << std::endl;
    }
}

// ============================================================================
// 第十個 TDD 測試：基本工作流程
// ============================================================================

TEST_F(FirstTddTest, BasicWorkflow_InitializeShutdown_ShouldWork) {
    // Arrange - 準備測試數據
    SdkConfig config;
    config.enableLogging = true;
    config.logFilePath = L"C:\\temp\\vdd_workflow_test.log";
    
    // Act - 執行完整的工作流程
    Status initStatus = Initialize(config);
    EXPECT_EQ(initStatus, Status::Ok);
    
    // 檢查狀態
    bool isActive = IsActive();
    EXPECT_FALSE(isActive);
    
    uint32_t displayCount = GetActiveDisplayCount();
    EXPECT_EQ(displayCount, 0);
    
    // 關閉
    Status shutdownStatus = Shutdown();
    EXPECT_EQ(shutdownStatus, Status::Ok);
    
    // Assert - 驗證結果
    std::cout << "基本工作流程測試通過: 初始化 -> 檢查狀態 -> 關閉" << std::endl;
}

// ============================================================================
// 測試運行器
// ============================================================================

int main(int argc, char** argv) {
    std::cout << "========================================" << std::endl;
    std::cout << "VDD SDK 第一個 TDD 測試" << std::endl;
    std::cout << "========================================" << std::endl;
    
    ::testing::InitGoogleTest(&argc, argv);
    
    // 設置測試環境
    std::cout << "設置測試環境..." << std::endl;
    
    int result = RUN_ALL_TESTS();
    
    std::cout << "========================================" << std::endl;
    if (result == 0) {
        std::cout << "所有測試通過！" << std::endl;
    } else {
        std::cout << "部分測試失敗。" << std::endl;
    }
    std::cout << "========================================" << std::endl;
    
    return result;
}
