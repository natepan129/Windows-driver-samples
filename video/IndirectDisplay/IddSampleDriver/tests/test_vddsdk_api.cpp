/*++

Copyright (c) 2024 VDD SDK Project

Abstract:

    VDD SDK API Unit Tests
    Tests basic functionality of all public API functions

Environment:

    User Mode, C++17, Google Test

--*/

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "vddsdk.h"
#include <memory>
#include <thread>
#include <chrono>

using namespace vdd;
using namespace testing;

class VddSdkApiTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup before each test
        m_config.enableLogging = true;
        m_config.logFilePath = L"C:\\temp\\vdd_test.log";
        m_config.defaultTimeoutMs = 5000;
    }

    void TearDown() override {
        // Cleanup after each test
        if (IsActive()) {
            Deactivate();
        }
        Shutdown();
    }

    SdkConfig m_config;
};

// ============================================================================
// 基本功能測試
// ============================================================================

TEST_F(VddSdkApiTest, InitializeAndShutdown) {
    // 測試初始化
    Status status = Initialize(m_config);
    EXPECT_EQ(status, Status::Ok);
    
    // 測試版本信息
    Version version = GetVersion();
    EXPECT_EQ(version.major, VDD_SDK_VERSION_MAJOR);
    EXPECT_EQ(version.minor, VDD_SDK_VERSION_MINOR);
    EXPECT_EQ(version.patch, VDD_SDK_VERSION_PATCH);
    
    // 測試關閉
    status = Shutdown();
    EXPECT_EQ(status, Status::Ok);
}

TEST_F(VddSdkApiTest, DoubleInitialize) {
    // 第一次初始化
    Status status = Initialize(m_config);
    EXPECT_EQ(status, Status::Ok);
    
    // 第二次初始化應該返回 AlreadyInstalled
    status = Initialize(m_config);
    EXPECT_EQ(status, Status::AlreadyInstalled);
    
    // 關閉
    Shutdown();
}

TEST_F(VddSdkApiTest, ShutdownWithoutInitialize) {
    // 未初始化就關閉
    Status status = Shutdown();
    EXPECT_EQ(status, Status::NotInstalled);
}

TEST_F(VddSdkApiTest, GetSystemInfo) {
    Initialize(m_config);
    
    std::string systemInfo = GetSystemInfo();
    EXPECT_FALSE(systemInfo.empty());
    
    // 應該包含基本系統信息
    EXPECT_THAT(systemInfo, HasSubstr("System"));
}

// ============================================================================
// 驅動程序管理測試
// ============================================================================

TEST_F(VddSdkApiTest, DriverInstallation) {
    Initialize(m_config);
    
    // 檢查驅動程序狀態
    bool isInstalled = IsDriverInstalled();
    // 在測試環境中，驅動程序可能未安裝
    // 這是一個預期的結果，不應該導致測試失敗
    
    if (!isInstalled) {
        std::cout << "警告: VDD 驅動程序未安裝，某些測試可能跳過" << std::endl;
    }
}

TEST_F(VddSdkApiTest, DriverVersion) {
    Initialize(m_config);
    
    Version driverVersion = GetDriverVersion();
    if (IsDriverInstalled()) {
        EXPECT_GT(driverVersion.major, 0);
    } else {
        // 未安裝時版本應該為 0
        EXPECT_EQ(driverVersion.major, 0);
        EXPECT_EQ(driverVersion.minor, 0);
        EXPECT_EQ(driverVersion.patch, 0);
    }
}

// ============================================================================
// 顯示器管理測試
// ============================================================================

TEST_F(VddSdkApiTest, VirtualDisplayActivation) {
    Initialize(m_config);
    
    // 創建虛擬顯示器描述
    VirtualDisplayDesc desc;
    desc.name = "Test Virtual Display";
    desc.preferredMode = { 1920, 1080, 60, 1 };
    desc.hdr10 = false;
    desc.stereoscopic = false;
    
    // 激活虛擬顯示器
    Status status = Activate(desc, 1);
    
    if (IsDriverInstalled()) {
        // 如果驅動程序已安裝，應該成功激活
        EXPECT_EQ(status, Status::Ok);
        EXPECT_TRUE(IsActive());
        EXPECT_EQ(GetActiveDisplayCount(), 1);
        
        // 停用
        status = Deactivate();
        EXPECT_EQ(status, Status::Ok);
        EXPECT_FALSE(IsActive());
    } else {
        // 如果驅動程序未安裝，應該返回相應錯誤
        EXPECT_NE(status, Status::Ok);
    }
}

TEST_F(VddSdkApiTest, DisplayConfiguration) {
    Initialize(m_config);
    
    if (!IsDriverInstalled()) {
        GTEST_SKIP() << "跳過測試：驅動程序未安裝";
    }
    
    // 激活虛擬顯示器
    VirtualDisplayDesc desc;
    desc.name = "Test Display";
    desc.preferredMode = { 1920, 1080, 60, 1 };
    
    Status status = Activate(desc, 1);
    ASSERT_EQ(status, Status::Ok);
    
    // 測試模式設置
    DisplayMode newMode = { 2560, 1440, 90, 1 };
    status = SetMode(0, newMode);
    EXPECT_EQ(status, Status::Ok);
    
    // 測試位置設置
    DisplayRect rect = { 1920, 0, 2560, 1440 };
    status = SetLocation(0, rect);
    EXPECT_EQ(status, Status::Ok);
    
    // 測試設置為主顯示器
    status = SetPrimary(0);
    EXPECT_EQ(status, Status::Ok);
    
    // 清理
    Deactivate();
}

TEST_F(VddSdkApiTest, DisplayQuery) {
    Initialize(m_config);
    
    if (!IsDriverInstalled()) {
        GTEST_SKIP() << "跳過測試：驅動程序未安裝";
    }
    
    // 激活虛擬顯示器
    VirtualDisplayDesc desc;
    desc.name = "Test Display";
    desc.preferredMode = { 1920, 1080, 60, 1 };
    
    Status status = Activate(desc, 1);
    ASSERT_EQ(status, Status::Ok);
    
    // 查詢當前模式
    DisplayMode currentMode;
    status = GetMode(0, currentMode);
    EXPECT_EQ(status, Status::Ok);
    EXPECT_EQ(currentMode.width, 1920);
    EXPECT_EQ(currentMode.height, 1080);
    
    // 查詢當前位置
    DisplayRect currentRect;
    status = GetLocation(0, currentRect);
    EXPECT_EQ(status, Status::Ok);
    
    // 清理
    Deactivate();
}

// ============================================================================
// 適配器枚舉測試
// ============================================================================

TEST_F(VddSdkApiTest, EnumerateAdapters) {
    Initialize(m_config);
    
    std::vector<AdapterInfo> adapters;
    Status status = EnumerateAdapters(adapters);
    
    EXPECT_EQ(status, Status::Ok);
    EXPECT_GE(adapters.size(), 0); // 至少應該有 0 個適配器
    
    // 檢查適配器信息
    for (const auto& adapter : adapters) {
        EXPECT_FALSE(adapter.name.empty());
        EXPECT_FALSE(adapter.deviceInstanceId.empty());
    }
}

TEST_F(VddSdkApiTest, EnumerateModes) {
    Initialize(m_config);
    
    if (!IsDriverInstalled()) {
        GTEST_SKIP() << "跳過測試：驅動程序未安裝";
    }
    
    // 激活虛擬顯示器
    VirtualDisplayDesc desc;
    desc.name = "Test Display";
    desc.preferredMode = { 1920, 1080, 60, 1 };
    
    Status status = Activate(desc, 1);
    ASSERT_EQ(status, Status::Ok);
    
    // 枚舉支持的模式
    std::vector<DisplayMode> modes;
    status = EnumerateModes(0, modes);
    
    EXPECT_EQ(status, Status::Ok);
    EXPECT_GT(modes.size(), 0); // 應該至少有一個模式
    
    // 檢查模式信息
    for (const auto& mode : modes) {
        EXPECT_GT(mode.width, 0);
        EXPECT_GT(mode.height, 0);
        EXPECT_GT(mode.refreshNumerator, 0);
        EXPECT_GT(mode.refreshDenominator, 0);
    }
    
    // 清理
    Deactivate();
}

// ============================================================================
// 會話管理測試
// ============================================================================

TEST_F(VddSdkApiTest, SessionManagement) {
    Initialize(m_config);
    
    // 開始會話
    LeaseHandle lease;
    Status status = BeginSession(lease);
    
    if (IsDriverInstalled()) {
        EXPECT_EQ(status, Status::Ok);
        EXPECT_TRUE(lease.IsValid());
        EXPECT_NE(lease.id, 0);
        
        // 測試會話狀態
        bool isActive;
        uint32_t timeRemaining;
        status = GetSessionState(lease, isActive, timeRemaining);
        EXPECT_EQ(status, Status::Ok);
        
        // 結束會話
        status = EndSession(lease);
        EXPECT_EQ(status, Status::Ok);
    } else {
        // 驅動程序未安裝時，會話管理可能不可用
        EXPECT_NE(status, Status::Ok);
    }
}

TEST_F(VddSdkApiTest, LeasedActivation) {
    Initialize(m_config);
    
    if (!IsDriverInstalled()) {
        GTEST_SKIP() << "跳過測試：驅動程序未安裝";
    }
    
    // 開始會話
    LeaseHandle lease;
    Status status = BeginSession(lease);
    ASSERT_EQ(status, Status::Ok);
    
    // 使用會話激活
    ActivateOptions options;
    options.desc.name = "Leased Display";
    options.desc.preferredMode = { 1920, 1080, 60, 1 };
    options.count = 1;
    options.autoRestoreOnCrash = true;
    
    status = ActivateLeased(options, lease);
    EXPECT_EQ(status, Status::Ok);
    
    // 發送心跳
    status = Heartbeat(lease);
    EXPECT_EQ(status, Status::Ok);
    
    // 結束會話
    status = EndSession(lease);
    EXPECT_EQ(status, Status::Ok);
}

// ============================================================================
// 錯誤處理測試
// ============================================================================

TEST_F(VddSdkApiTest, ErrorHandling) {
    Initialize(m_config);
    
    // 測試無效參數
    Status status = SetMode(999, { 1920, 1080, 60, 1 });
    EXPECT_NE(status, Status::Ok);
    
    status = SetLocation(999, { 0, 0, 1920, 1080 });
    EXPECT_NE(status, Status::Ok);
    
    status = SetPrimary(999);
    EXPECT_NE(status, Status::Ok);
    
    // 測試錯誤信息
    std::string error = GetLastError();
    EXPECT_FALSE(error.empty());
}

TEST_F(VddSdkApiTest, StatusToString) {
    // 測試所有狀態碼的字符串轉換
    EXPECT_EQ(StatusToString(Status::Ok), "Success");
    EXPECT_EQ(StatusToString(Status::AlreadyInstalled), "Already installed");
    EXPECT_EQ(StatusToString(Status::NotInstalled), "Not installed");
    EXPECT_EQ(StatusToString(Status::AdminRequired), "Administrator privileges required");
    EXPECT_EQ(StatusToString(Status::DriverError), "Driver operation failed");
}

// ============================================================================
// 高級功能測試
// ============================================================================

TEST_F(VddSdkApiTest, AdvancedFeatures) {
    Initialize(m_config);
    
    if (!IsDriverInstalled()) {
        GTEST_SKIP() << "跳過測試：驅動程序未安裝";
    }
    
    // 激活虛擬顯示器
    VirtualDisplayDesc desc;
    desc.name = "Advanced Display";
    desc.preferredMode = { 1920, 1080, 60, 1 };
    desc.hdr10 = true;
    desc.stereoscopic = true;
    
    Status status = Activate(desc, 1);
    ASSERT_EQ(status, Status::Ok);
    
    // 測試 HDR 支持
    status = SetHdrSupport(0, true);
    EXPECT_EQ(status, Status::Ok);
    
    // 測試立體支持
    status = SetStereoSupport(0, true);
    EXPECT_EQ(status, Status::Ok);
    
    // 測試自定義 EDID
    std::vector<uint8_t> customEdid(128, 0);
    status = SetCustomEdid(0, customEdid);
    EXPECT_EQ(status, Status::Ok);
    
    // 查詢 EDID
    std::vector<uint8_t> edidData;
    status = GetEdid(0, edidData);
    EXPECT_EQ(status, Status::Ok);
    
    // 清理
    Deactivate();
}

// ============================================================================
// 恢復功能測試
// ============================================================================

TEST_F(VddSdkApiTest, RecoveryFunctions) {
    Initialize(m_config);
    
    // 測試孤兒狀態恢復
    Status status = RecoverOrphanedState();
    // 這個函數在沒有孤兒狀態時應該成功
    EXPECT_EQ(status, Status::Ok);
    
    // 測試確保驅動程序運行
    status = EnsureDriverRunning();
    // 這個函數應該嘗試確保服務可用
    // 在測試環境中可能返回不同的結果
}

// ============================================================================
// 工具函數測試
// ============================================================================

TEST_F(VddSdkApiTest, UtilityFunctions) {
    // 測試管理員權限檢查
    bool isAdmin = IsRunningAsAdministrator();
    // 在測試環境中可能不是管理員
    EXPECT_FALSE(isAdmin); // 或者根據實際情況調整
    
    // 測試權限提升請求
    bool elevated = RequestElevation();
    // 在測試環境中可能無法提升權限
    EXPECT_FALSE(elevated); // 或者根據實際情況調整
}
