#include "vddsdk.h"
#include <iostream>
#include <windows.h>

int main() {
    std::cout << "=== VDD SDK Rollback Test ===" << std::endl << std::endl;
    
    // Test 1: 使用无效路径（应该立即失败，无残留）
    std::cout << "Test 1: Invalid INF path (should fail immediately)" << std::endl;
    vdd::Status status = vdd::InstallDriver(L"nonexistent.inf");
    std::cout << "  Status: " << vdd::StatusToString(status) << std::endl;
    std::cout << "  Error: " << vdd::GetLastError() << std::endl;
    
    // 验证无残留
    std::cout << "  Verifying no zombie devices..." << std::endl;
    system("powershell -Command \"Get-PnpDevice | Where-Object { $_.FriendlyName -like '*IddSampleDriver*' }\"");
    std::cout << "  (No output = no devices, PASS)" << std::endl << std::endl;
    
    // Test 2: 使用正确的 INF（应该成功）
    std::cout << "Test 2: Valid INF path (should succeed)" << std::endl;
    status = vdd::InstallDriver(L"x64\\Release\\IddSampleDriver.inf");
    std::cout << "  Status: " << vdd::StatusToString(status) << std::endl;
    if (status != vdd::Status::Ok) {
        std::cout << "  Error: " << vdd::GetLastError() << std::endl;
    }
    
    // 验证设备存在
    std::cout << "  Verifying device created..." << std::endl;
    system("powershell -Command \"Get-PnpDevice | Where-Object { $_.FriendlyName -like '*IddSampleDriver*' }\"");
    std::cout << std::endl;
    
    // Test 3: 卸载
    std::cout << "Test 3: Uninstall driver" << std::endl;
    status = vdd::UninstallDriver();
    std::cout << "  Status: " << vdd::StatusToString(status) << std::endl;
    if (status != vdd::Status::Ok) {
        std::cout << "  Error: " << vdd::GetLastError() << std::endl;
    }
    
    // 验证清理
    std::cout << "  Verifying cleanup..." << std::endl;
    system("powershell -Command \"Get-PnpDevice | Where-Object { $_.FriendlyName -like '*IddSampleDriver*' }\"");
    std::cout << "  (No output = cleaned, PASS)" << std::endl << std::endl;
    
    std::cout << "=== Test Complete ===" << std::endl;
    return 0;
}

