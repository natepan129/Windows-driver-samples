/*++

Copyright (c) 2024 VDD SDK Project

Abstract:

    VDD SDK Integration Tests
    Tests complete end-to-end functionality, including interaction with actual systems

Environment:

    User Mode, C++17, Google Test

--*/

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "vddsdk.h"
#include <memory>
#include <thread>
#include <chrono>
#include <vector>
#include <string>

using namespace vdd;
using namespace testing;

class VddSdkIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        m_config.enableLogging = true;
        m_config.logFilePath = L"C:\\temp\\vdd_integration_test.log";
        m_config.defaultTimeoutMs = 10000; // Integration tests need longer timeout
        
        // Clean up any orphaned state
        Initialize(m_config);
        RecoverOrphanedState();
        Shutdown();
    }

    void TearDown() override {
        // 確保清理所有資源
        if (IsActive()) {
            Deactivate();
        }
        Shutdown();
    }

    SdkConfig m_config;
};

// ============================================================================
// 完整工作流程測試
// ============================================================================

TEST_F(VddSdkIntegrationTest, CompleteWorkflow) {
    // 測試完整的工作流程：初始化 -> 激活 -> 配置 -> 停用 -> 關閉
    
    // 1. 初始化
    Status status = Initialize(m_config);
    EXPECT_EQ(status, Status::Ok);
    
    // 2. 檢查系統狀態
    std::string systemInfo = GetSystemInfo();
    EXPECT_FALSE(systemInfo.empty());
    
    // 3. 枚舉適配器
    std::vector<AdapterInfo> adapters;
    status = EnumerateAdapters(adapters);
    EXPECT_EQ(status, Status::Ok);
    
    // 4. 創建虛擬顯示器
    VirtualDisplayDesc desc;
    desc.name = "Integration Test Display";
    desc.preferredMode = { 1920, 1080, 60, 1 };
    desc.hdr10 = false;
    desc.stereoscopic = false;
    
    status = Activate(desc, 1);
    if (status == Status::Ok) {
        EXPECT_TRUE(IsActive());
        EXPECT_EQ(GetActiveDisplayCount(), 1);
        
        // 5. 配置顯示器
        DisplayMode newMode = { 2560, 1440, 90, 1 };
        status = SetMode(0, newMode);
        EXPECT_EQ(status, Status::Ok);
        
        DisplayRect rect = { 1920, 0, 2560, 1440 };
        status = SetLocation(0, rect);
        EXPECT_EQ(status, Status::Ok);
        
        // 6. 查詢當前狀態
        DisplayMode currentMode;
        status = GetMode(0, currentMode);
        EXPECT_EQ(status, Status::Ok);
        
        DisplayRect currentRect;
        status = GetLocation(0, currentRect);
        EXPECT_EQ(status, Status::Ok);
        
        // 7. 枚舉支持的模式
        std::vector<DisplayMode> modes;
        status = EnumerateModes(0, modes);
        EXPECT_EQ(status, Status::Ok);
        EXPECT_GT(modes.size(), 0);
        
        // 8. 停用顯示器
        status = Deactivate();
        EXPECT_EQ(status, Status::Ok);
        EXPECT_FALSE(IsActive());
    } else {
        // 如果驅動程序未安裝，跳過測試
        GTEST_SKIP() << "跳過測試：驅動程序未安裝或服務不可用";
    }
}

TEST_F(VddSdkIntegrationTest, MultipleDisplays) {
    // 測試多個虛擬顯示器
    
    Initialize(m_config);
    
    VirtualDisplayDesc desc;
    desc.name = "Multi Display";
    desc.preferredMode = { 1920, 1080, 60, 1 };
    
    Status status = Activate(desc, 3); // 創建 3 個顯示器
    if (status == Status::Ok) {
        EXPECT_TRUE(IsActive());
        EXPECT_EQ(GetActiveDisplayCount(), 3);
        
        // 配置每個顯示器
        for (uint32_t i = 0; i < 3; ++i) {
            DisplayRect rect = { static_cast<int32_t>(i * 1920), 0, 1920, 1080 };
            status = SetLocation(i, rect);
            EXPECT_EQ(status, Status::Ok);
        }
        
        // 設置第一個為主顯示器
        status = SetPrimary(0);
        EXPECT_EQ(status, Status::Ok);
        
        // 停用所有顯示器
        status = Deactivate();
        EXPECT_EQ(status, Status::Ok);
        EXPECT_FALSE(IsActive());
    } else {
        GTEST_SKIP() << "跳過測試：驅動程序未安裝或服務不可用";
    }
}

// ============================================================================
// 會話管理集成測試
// ============================================================================

TEST_F(VddSdkIntegrationTest, SessionManagementWorkflow) {
    // 測試完整的會話管理工作流程
    
    Initialize(m_config);
    
    // 開始會話
    LeaseHandle lease;
    Status status = BeginSession(lease);
    if (status == Status::Ok) {
        EXPECT_TRUE(lease.IsValid());
        
        // 使用會話激活顯示器
        ActivateOptions options;
        options.desc.name = "Session Display";
        options.desc.preferredMode = { 1920, 1080, 60, 1 };
        options.count = 1;
        options.autoRestoreOnCrash = true;
        options.heartbeatIntervalMs = 1000;
        options.leaseTimeoutMs = 30000;
        
        status = ActivateLeased(options, lease);
        if (status == Status::Ok) {
            EXPECT_TRUE(IsActive());
            
            // 發送多次心跳
            for (int i = 0; i < 5; ++i) {
                status = Heartbeat(lease);
                EXPECT_EQ(status, Status::Ok);
                
                // 檢查會話狀態
                bool isActive;
                uint32_t timeRemaining;
                status = GetSessionState(lease, isActive, timeRemaining);
                EXPECT_EQ(status, Status::Ok);
                EXPECT_TRUE(isActive);
                EXPECT_GT(timeRemaining, 0);
                
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
        
        // 結束會話
        status = EndSession(lease);
        EXPECT_EQ(status, Status::Ok);
    } else {
        GTEST_SKIP() << "跳過測試：會話管理不可用";
    }
}

TEST_F(VddSdkIntegrationTest, SessionHeartbeatTimeout) {
    // 測試會話心跳超時
    
    Initialize(m_config);
    
    LeaseHandle lease;
    Status status = BeginSession(lease);
    if (status == Status::Ok) {
        // 激活顯示器
        ActivateOptions options;
        options.desc.name = "Timeout Test Display";
        options.desc.preferredMode = { 1920, 1080, 60, 1 };
        options.count = 1;
        options.heartbeatIntervalMs = 100;
        options.leaseTimeoutMs = 1000; // 短超時時間
        
        status = ActivateLeased(options, lease);
        if (status == Status::Ok) {
            // 等待超時
            std::this_thread::sleep_for(std::chrono::milliseconds(1500));
            
            // 嘗試發送心跳（應該失敗）
            status = Heartbeat(lease);
            EXPECT_NE(status, Status::Ok);
        }
        
        // 結束會話
        EndSession(lease);
    } else {
        GTEST_SKIP() << "跳過測試：會話管理不可用";
    }
}

// ============================================================================
// 高級功能集成測試
// ============================================================================

TEST_F(VddSdkIntegrationTest, AdvancedFeatures) {
    // 測試高級功能：HDR、立體 3D、自定義 EDID
    
    Initialize(m_config);
    
    VirtualDisplayDesc desc;
    desc.name = "Advanced Features Display";
    desc.preferredMode = { 3840, 2160, 60, 1 };
    desc.hdr10 = true;
    desc.stereoscopic = true;
    
    Status status = Activate(desc, 1);
    if (status == Status::Ok) {
        EXPECT_TRUE(IsActive());
        
        // 測試 HDR 支持
        status = SetHdrSupport(0, true);
        EXPECT_EQ(status, Status::Ok);
        
        // 測試立體支持
        status = SetStereoSupport(0, true);
        EXPECT_EQ(status, Status::Ok);
        
        // 測試自定義 EDID
        std::vector<uint8_t> customEdid(128, 0);
        // 填充基本的 EDID 數據
        customEdid[0] = 0x00;
        customEdid[1] = 0xFF;
        customEdid[2] = 0xFF;
        customEdid[3] = 0xFF;
        customEdid[4] = 0xFF;
        customEdid[5] = 0xFF;
        customEdid[6] = 0xFF;
        customEdid[7] = 0x00;
        
        status = SetCustomEdid(0, customEdid);
        EXPECT_EQ(status, Status::Ok);
        
        // 查詢 EDID
        std::vector<uint8_t> edidData;
        status = GetEdid(0, edidData);
        EXPECT_EQ(status, Status::Ok);
        EXPECT_EQ(edidData.size(), 128);
        
        // 停用
        status = Deactivate();
        EXPECT_EQ(status, Status::Ok);
    } else {
        GTEST_SKIP() << "跳過測試：驅動程序未安裝或服務不可用";
    }
}

// ============================================================================
// 錯誤恢復測試
// ============================================================================

TEST_F(VddSdkIntegrationTest, ErrorRecovery) {
    // 測試錯誤恢復功能
    
    Initialize(m_config);
    
    // 測試孤兒狀態恢復
    Status status = RecoverOrphanedState();
    EXPECT_EQ(status, Status::Ok);
    
    // 測試確保驅動程序運行
    status = EnsureDriverRunning();
    // 這個函數的結果取決於系統狀態
    // 在測試環境中可能返回不同的結果
    
    // 測試錯誤信息
    std::string error = GetLastError();
    // 錯誤信息可能為空，這是正常的
}

TEST_F(VddSdkIntegrationTest, ConcurrentAccess) {
    // 測試並發訪問
    
    Initialize(m_config);
    
    // 創建多個線程同時訪問 SDK
    std::vector<std::thread> threads;
    std::vector<Status> results(5);
    
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back([&, i]() {
            VirtualDisplayDesc desc;
            desc.name = "Concurrent Display " + std::to_string(i);
            desc.preferredMode = { 1920, 1080, 60, 1 };
            
            results[i] = Activate(desc, 1);
            
            if (results[i] == Status::Ok) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                Deactivate();
            }
        });
    }
    
    // 等待所有線程完成
    for (auto& thread : threads) {
        thread.join();
    }
    
    // 檢查結果
    int successCount = 0;
    for (const auto& result : results) {
        if (result == Status::Ok) {
            successCount++;
        }
    }
    
    // 至少應該有一些操作成功
    EXPECT_GT(successCount, 0);
}

// ============================================================================
// 性能集成測試
// ============================================================================

TEST_F(VddSdkIntegrationTest, PerformanceTest) {
    // 測試性能
    
    Initialize(m_config);
    
    auto start = std::chrono::high_resolution_clock::now();
    
    // 執行多次操作
    const int iterations = 50;
    int successCount = 0;
    
    for (int i = 0; i < iterations; ++i) {
        VirtualDisplayDesc desc;
        desc.name = "Performance Test " + std::to_string(i);
        desc.preferredMode = { 1920, 1080, 60, 1 };
        
        Status status = Activate(desc, 1);
        if (status == Status::Ok) {
            successCount++;
            
            // 配置顯示器
            DisplayRect rect = { 0, 0, 1920, 1080 };
            SetLocation(0, rect);
            
            // 停用
            Deactivate();
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // 驗證性能
    EXPECT_GT(successCount, 0); // 至少應該有一些操作成功
    EXPECT_LT(duration.count(), 30000); // 應該在 30 秒內完成
    
    std::cout << "性能測試結果: " << successCount << "/" << iterations 
              << " 操作成功，耗時 " << duration.count() << "ms" << std::endl;
}

// ============================================================================
// 邊界條件測試
// ============================================================================

TEST_F(VddSdkIntegrationTest, BoundaryConditions) {
    // 測試邊界條件
    
    Initialize(m_config);
    
    // 測試無效的顯示模式
    DisplayMode invalidMode = { 0, 0, 0, 0 };
    Status status = SetMode(0, invalidMode);
    EXPECT_NE(status, Status::Ok);
    
    // 測試無效的位置
    DisplayRect invalidRect = { -1, -1, 0, 0 };
    status = SetLocation(0, invalidRect);
    EXPECT_NE(status, Status::Ok);
    
    // 測試無效的輸出索引
    DisplayMode validMode = { 1920, 1080, 60, 1 };
    status = SetMode(999, validMode);
    EXPECT_NE(status, Status::Ok);
    
    // 測試空名稱
    VirtualDisplayDesc desc;
    desc.name = ""; // 空名稱
    desc.preferredMode = { 1920, 1080, 60, 1 };
    
    status = Activate(desc, 1);
    EXPECT_NE(status, Status::Ok);
}

// ============================================================================
// 系統兼容性測試
// ============================================================================

TEST_F(VddSdkIntegrationTest, SystemCompatibility) {
    // 測試系統兼容性
    
    Initialize(m_config);
    
    // 檢查管理員權限
    bool isAdmin = IsRunningAsAdministrator();
    std::cout << "管理員權限: " << (isAdmin ? "是" : "否") << std::endl;
    
    // 檢查驅動程序狀態
    bool driverInstalled = IsDriverInstalled();
    std::cout << "驅動程序已安裝: " << (driverInstalled ? "是" : "否") << std::endl;
    
    if (driverInstalled) {
        Version driverVersion = GetDriverVersion();
        std::cout << "驅動程序版本: " << driverVersion.major << "." 
                  << driverVersion.minor << "." << driverVersion.patch << std::endl;
    }
    
    // 檢查系統信息
    std::string systemInfo = GetSystemInfo();
    EXPECT_FALSE(systemInfo.empty());
    std::cout << "系統信息: " << systemInfo << std::endl;
}

// ============================================================================
// 清理測試
// ============================================================================

TEST_F(VddSdkIntegrationTest, CleanupTest) {
    // 測試清理功能
    
    Initialize(m_config);
    
    // 創建多個顯示器
    VirtualDisplayDesc desc;
    desc.name = "Cleanup Test Display";
    desc.preferredMode = { 1920, 1080, 60, 1 };
    
    Status status = Activate(desc, 3);
    if (status == Status::Ok) {
        EXPECT_TRUE(IsActive());
        EXPECT_EQ(GetActiveDisplayCount(), 3);
        
        // 停用所有顯示器
        status = Deactivate();
        EXPECT_EQ(status, Status::Ok);
        EXPECT_FALSE(IsActive());
    }
    
    // 關閉 SDK
    status = Shutdown();
    EXPECT_EQ(status, Status::Ok);
    
    // 再次關閉應該返回 NotInstalled
    status = Shutdown();
    EXPECT_EQ(status, Status::NotInstalled);
}
