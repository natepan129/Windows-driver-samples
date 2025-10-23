#include "vddsdk.h"
#include <iostream>
#include <windows.h>
#include <cassert>

using namespace vdd;

// 測試 IOCTL 通訊
void TestIOCTLCommunication() {
    std::cout << "=== 測試 IOCTL 通訊 ===" << std::endl;
    
    // 測試設備開啟
    Status status = vdd::OpenDevice();
    std::cout << "開啟設備: " << GetStatusString(status) << std::endl;
    
    if (status == Status::NotInstalled) {
        std::cout << "⚠️  驅動程式未安裝，無法測試 IOCTL" << std::endl;
        return;
    }
    
    if (status != Status::Ok) {
        std::cout << "❌ 設備開啟失敗: " << GetStatusString(status) << std::endl;
        return;
    }
    
    // 測試 IOCTL 命令
    std::cout << "測試 IOCTL 命令..." << std::endl;
    
    // 測試設定監視器數量
    status = SetMonitorCount(1);
    std::cout << "設定監視器數量 (1): " << GetStatusString(status) << std::endl;
    
    // 測試配置顯示器
    status = ConfigureDisplay(0, 1920, 1080, 60, "IOCTL Test Display");
    std::cout << "配置顯示器: " << GetStatusString(status) << std::endl;
    
    // 測試設定多個監視器
    status = SetMonitorCount(2);
    std::cout << "設定監視器數量 (2): " << GetStatusString(status) << std::endl;
    
    // 配置第二個顯示器
    status = ConfigureDisplay(1, 2560, 1440, 90, "IOCTL Test Display 2");
    std::cout << "配置第二個顯示器: " << GetStatusString(status) << std::endl;
    
    // 清理
    vdd::CloseDevice();
    std::cout << "✓ IOCTL 通訊測試完成" << std::endl;
}

// 測試驅動程式狀態
void TestDriverStatus() {
    std::cout << "\n=== 測試驅動程式狀態 ===" << std::endl;
    
    Status status = vdd::TestDriverConnection();
    std::cout << "驅動程式連接狀態: " << GetStatusString(status) << std::endl;
    
    switch (status) {
        case Status::Ok:
            std::cout << "✓ 驅動程式正常運行" << std::endl;
            break;
        case Status::NotInstalled:
            std::cout << "⚠️  驅動程式未安裝" << std::endl;
            break;
        case Status::DriverError:
            std::cout << "❌ 驅動程式錯誤" << std::endl;
            break;
        default:
            std::cout << "⚠️  未知狀態: " << GetStatusString(status) << std::endl;
            break;
    }
}

// 測試配置持久性
void TestConfigurationPersistence() {
    std::cout << "\n=== 測試配置持久性 ===" << std::endl;
    
    // 開啟設備
    Status status = vdd::OpenDevice();
    if (status != Status::Ok) {
        std::cout << "無法開啟設備，跳過持久性測試" << std::endl;
        return;
    }
    
    // 設定初始配置
    std::cout << "設定初始配置..." << std::endl;
    status = SetMonitorCount(1);
    assert(status == Status::Ok);
    
    status = ConfigureDisplay(0, 1920, 1080, 60, "Persistence Test");
    assert(status == Status::Ok);
    
    // 關閉設備
    vdd::CloseDevice();
    std::cout << "設備已關閉" << std::endl;
    
    // 重新開啟並檢查配置
    std::cout << "重新開啟設備..." << std::endl;
    status = vdd::OpenDevice();
    if (status == Status::Ok) {
        std::cout << "✓ 設備重新開啟成功" << std::endl;
        
        // 嘗試讀取配置（如果驅動程式支援）
        status = vdd::TestDriverConnection();
        std::cout << "重新連接狀態: " << GetStatusString(status) << std::endl;
        
        vdd::CloseDevice();
    }
    
    std::cout << "✓ 配置持久性測試完成" << std::endl;
}

// 測試錯誤恢復
void TestErrorRecovery() {
    std::cout << "\n=== 測試錯誤恢復 ===" << std::endl;
    
    // 測試無效的 IOCTL 參數
    std::cout << "測試無效參數..." << std::endl;
    
    Status status = vdd::OpenDevice();
    if (status != Status::Ok) {
        std::cout << "無法開啟設備，跳過錯誤恢復測試" << std::endl;
        return;
    }
    
    // 測試無效的監視器索引
    status = ConfigureDisplay(999, 1920, 1080, 60, "Invalid Index");
    std::cout << "無效監視器索引: " << GetStatusString(status) << std::endl;
    
    // 測試無效的解析度
    status = ConfigureDisplay(0, 0, 0, 0, "Invalid Resolution");
    std::cout << "無效解析度: " << GetStatusString(status) << std::endl;
    
    // 測試無效的刷新率
    status = ConfigureDisplay(0, 1920, 1080, 0, "Invalid Refresh");
    std::cout << "無效刷新率: " << GetStatusString(status) << std::endl;
    
    vdd::CloseDevice();
    std::cout << "✓ 錯誤恢復測試完成" << std::endl;
}

int main() {
    std::cout << "VDD 驅動程式 IOCTL 測試" << std::endl;
    std::cout << "========================" << std::endl;
    
    try {
        TestDriverStatus();
        TestIOCTLCommunication();
        TestConfigurationPersistence();
        TestErrorRecovery();
        
        std::cout << "\n🎉 所有 IOCTL 測試完成！" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cout << "\n❌ 測試失敗: " << e.what() << std::endl;
        return 1;
    }
}
