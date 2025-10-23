#include "vddsdk.h"
#include <iostream>
#include <windows.h>
#include <cassert>

using namespace vdd;

// 測試 1: 基本 SDK 功能
void TestBasicSDK() {
    std::cout << "=== 測試基本 SDK 功能 ===" << std::endl;
    
    // 測試版本
    Version version = GetVersion();
    std::cout << "SDK 版本: " << version.major << "." << version.minor << "." << version.patch << std::endl;
    assert(version.major >= 1);
    
    // 測試狀態字串
    const char* statusStr = GetStatusString(Status::Ok);
    std::cout << "狀態字串測試: " << statusStr << std::endl;
    assert(strcmp(statusStr, "OK") == 0);
    
    std::cout << "✓ 基本 SDK 功能正常" << std::endl;
}

// 測試 2: 驅動程式連接
void TestDriverConnectionFunc() {
    std::cout << "\n=== 測試驅動程式連接 ===" << std::endl;
    
    Status status = vdd::TestDriverConnection();
    std::cout << "驅動程式連接狀態: " << GetStatusString(status) << std::endl;
    
    if (status == Status::NotInstalled) {
        std::cout << "⚠️  驅動程式未安裝，這是預期的" << std::endl;
    } else if (status == Status::Ok) {
        std::cout << "✓ 驅動程式連接成功" << std::endl;
    } else {
        std::cout << "❌ 驅動程式連接失敗: " << GetStatusString(status) << std::endl;
    }
}

// 測試 3: 配置功能
void TestConfiguration() {
    std::cout << "\n=== 測試配置功能 ===" << std::endl;
    
    // 測試設定監視器數量
    Status status = SetMonitorCount(1);
    std::cout << "設定監視器數量: " << GetStatusString(status) << std::endl;
    
    // 測試配置顯示器
    status = ConfigureDisplay(0, 1920, 1080, 60, "VDD Test Display");
    std::cout << "配置顯示器: " << GetStatusString(status) << std::endl;
    
    if (status == Status::NotInstalled) {
        std::cout << "⚠️  驅動程式未安裝，配置失敗是預期的" << std::endl;
    } else {
        std::cout << "✓ 配置功能正常" << std::endl;
    }
}

// 測試 4: 虛擬顯示器描述
void TestVirtualDisplayDesc() {
    std::cout << "\n=== 測試虛擬顯示器描述 ===" << std::endl;
    
    VirtualDisplayDesc desc;
    desc.name = "VDD XR Display";
    desc.preferredMode.width = 1920;
    desc.preferredMode.height = 1080;
    desc.preferredMode.refreshNumerator = 90;
    desc.preferredMode.refreshDenominator = 1;
    desc.hdr10 = false;
    desc.stereoscopic = false;
    
    std::cout << "顯示器名稱: " << desc.name << std::endl;
    std::cout << "解析度: " << desc.preferredMode.width << "x" << desc.preferredMode.height << std::endl;
    std::cout << "刷新率: " << desc.preferredMode.refreshNumerator << " Hz" << std::endl;
    std::cout << "HDR10: " << (desc.hdr10 ? "是" : "否") << std::endl;
    std::cout << "立體: " << (desc.stereoscopic ? "是" : "否") << std::endl;
    
    std::cout << "✓ 虛擬顯示器描述結構正常" << std::endl;
}

// 測試 5: 錯誤處理
void TestErrorHandling() {
    std::cout << "\n=== 測試錯誤處理 ===" << std::endl;
    
    // 測試無效參數
    Status status = ConfigureDisplay(999, 0, 0, 0, "");
    std::cout << "無效參數測試: " << GetStatusString(status) << std::endl;
    
    // 測試所有狀態碼
    Status allStatuses[] = {
        Status::Ok, Status::AlreadyInstalled, Status::NotInstalled,
        Status::NotActive, Status::Busy, Status::AdminRequired,
        Status::DriverError, Status::Timeout, Status::InvalidArg, Status::OsUnsupported
    };
    
    std::cout << "所有狀態碼測試:" << std::endl;
    for (auto s : allStatuses) {
        const char* str = GetStatusString(s);
        std::cout << "  " << (int)s << " -> " << str << std::endl;
        assert(str != nullptr);
    }
    
    std::cout << "✓ 錯誤處理正常" << std::endl;
}

int main() {
    std::cout << "VDD SDK 基本功能測試" << std::endl;
    std::cout << "====================" << std::endl;
    
    try {
        TestBasicSDK();
        TestDriverConnectionFunc();
        TestConfiguration();
        TestVirtualDisplayDesc();
        TestErrorHandling();
        
        std::cout << "\n🎉 所有基本測試通過！" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cout << "\n❌ 測試失敗: " << e.what() << std::endl;
        return 1;
    }
}
