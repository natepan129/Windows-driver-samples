// 最佳实践：结合简单方法 + 完整回滚
// 基于 setupapi_install_simple.cpp（已验证可用）+ 回滚机制
#include <windows.h>
#include <setupapi.h>
#include <newdev.h>
#include <cfgmgr32.h>
#include <stdio.h>
#include <string>

#pragma comment(lib, "setupapi.lib")
#pragma comment(lib, "newdev.lib")
#pragma comment(lib, "cfgmgr32.lib")

// 安装状态记录
struct InstallState {
    bool deviceCreated;
    bool deviceRegistered;
    bool driverInstalled;
    std::wstring deviceInstanceId;
    HDEVINFO deviceInfoSet;
    SP_DEVINFO_DATA devInfoData;
    
    InstallState() : deviceCreated(false), deviceRegistered(false), 
                     driverInstalled(false), deviceInfoSet(INVALID_HANDLE_VALUE) {
        devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
    }
};

// 回滚操作
bool RollbackInstallation(InstallState& state) {
    wprintf(L"\n========================================\n");
    wprintf(L"執行自動回滾\n");
    wprintf(L"========================================\n\n");
    
    bool success = true;
    
    // 如果设备已注册，移除它
    if (state.deviceRegistered && state.deviceInfoSet != INVALID_HANDLE_VALUE) {
        wprintf(L"[回滾] 移除已註冊的設備...\n");
        
        SP_REMOVEDEVICE_PARAMS removeParams = {};
        removeParams.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
        removeParams.ClassInstallHeader.InstallFunction = DIF_REMOVE;
        removeParams.Scope = DI_REMOVEDEVICE_GLOBAL;
        removeParams.HwProfile = 0;
        
        if (SetupDiSetClassInstallParamsW(
                state.deviceInfoSet,
                &state.devInfoData,
                (PSP_CLASSINSTALL_HEADER)&removeParams,
                sizeof(removeParams))) {
            
            if (SetupDiCallClassInstaller(DIF_REMOVE, state.deviceInfoSet, &state.devInfoData)) {
                wprintf(L"  ✓ 設備已移除\n");
            } else {
                wprintf(L"  ✗ 移除失敗 (錯誤: %d)\n", GetLastError());
                success = false;
            }
        }
    }
    
    // 清理设备信息集
    if (state.deviceInfoSet != INVALID_HANDLE_VALUE) {
        SetupDiDestroyDeviceInfoList(state.deviceInfoSet);
        state.deviceInfoSet = INVALID_HANDLE_VALUE;
    }
    
    wprintf(L"\n回滾完成\n");
    return success;
}

// 主安装函数 - 使用简单方法的逻辑
bool InstallDriver(const wchar_t* fullPath, InstallState& state) {
    wprintf(L"\n========================================\n");
    wprintf(L"開始安裝驅動程式\n");
    wprintf(L"========================================\n\n");
    
    wprintf(L"INF 文件: %s\n\n", fullPath);
    
    // 显示适配器类别 GUID
    GUID displayClassGuid = { 0x4D36E968, 0xE325, 0x11CE, 
        { 0xBF, 0xC1, 0x08, 0x00, 0x2B, 0xE1, 0x03, 0x18 } };
    
    // 步骤 1: 创建设备信息列表
    wprintf(L"[1/5] 創建設備信息列表...\n");
    state.deviceInfoSet = SetupDiCreateDeviceInfoList(&displayClassGuid, NULL);
    
    if (state.deviceInfoSet == INVALID_HANDLE_VALUE) {
        wprintf(L"  ✗ 失敗 (錯誤: %d)\n", GetLastError());
        return false;
    }
    wprintf(L"  ✓ 成功\n");
    
    // 步骤 2: 创建设备信息
    wprintf(L"[2/5] 創建設備信息...\n");
    
    if (!SetupDiCreateDeviceInfoW(
            state.deviceInfoSet,
            L"IddSampleDriver",
            &displayClassGuid,
            L"IddSampleDriver Device",
            NULL,
            DICD_GENERATE_ID,
            &state.devInfoData)) {
        wprintf(L"  ✗ 失敗 (錯誤: %d)\n", GetLastError());
        return false;
    }
    
    state.deviceCreated = true;
    wprintf(L"  ✓ 成功\n");
    
    // 步骤 3: 设置硬件 ID
    wprintf(L"[3/5] 設置硬體 ID...\n");
    
    wchar_t hardwareId[] = L"ROOT\\IddSampleDriver\0\0";
    
    if (!SetupDiSetDeviceRegistryPropertyW(
            state.deviceInfoSet,
            &state.devInfoData,
            SPDRP_HARDWAREID,
            (BYTE*)hardwareId,
            sizeof(hardwareId))) {
        wprintf(L"  ✗ 失敗 (錯誤: %d)\n", GetLastError());
        return false;
    }
    wprintf(L"  ✓ 成功\n");
    
    // 步骤 4: 注册设备（关键点 - 之后失败需要回滚）
    wprintf(L"[4/5] 註冊設備...\n");
    
    if (!SetupDiCallClassInstaller(DIF_REGISTERDEVICE, state.deviceInfoSet, &state.devInfoData)) {
        wprintf(L"  ✗ 失敗 (錯誤: %d)\n", GetLastError());
        return false;
    }
    
    state.deviceRegistered = true;  // 标记已注册 - 之后失败必须回滚
    
    // 获取设备实例 ID
    wchar_t deviceId[MAX_PATH] = {};
    if (SetupDiGetDeviceInstanceIdW(state.deviceInfoSet, &state.devInfoData, deviceId, MAX_PATH, NULL)) {
        state.deviceInstanceId = deviceId;
        wprintf(L"  ✓ 成功 (ID: %s)\n", deviceId);
    } else {
        wprintf(L"  ✓ 成功\n");
    }
    
    // 步骤 5: 安装驱动程序
    // ⚠️ 关键：使用简单方法，不设置额外参数！
    wprintf(L"[5/5] 安裝驅動程式...\n");
    
    if (!SetupDiCallClassInstaller(DIF_INSTALLDEVICE, state.deviceInfoSet, &state.devInfoData)) {
        DWORD error = GetLastError();
        wprintf(L"  ⚠ DIF_INSTALLDEVICE 失敗 (錯誤: %d)\n", error);
        wprintf(L"  嘗試使用備用方法...\n");
        
        // 备用方法：UpdateDriverForPlugAndPlayDevices
        // 这个方法会自动设置 Class！
        BOOL reboot = FALSE;
        if (UpdateDriverForPlugAndPlayDevicesW(
                NULL,
                L"ROOT\\IddSampleDriver",
                fullPath,
                INSTALLFLAG_FORCE,
                &reboot)) {
            wprintf(L"  ✓ 成功 (使用備用方法)\n");
            state.driverInstalled = true;
            
            if (reboot) {
                wprintf(L"  ℹ 可能需要重新啟動\n");
            }
        } else {
            wprintf(L"  ✗ 備用方法也失敗 (錯誤: %d)\n", GetLastError());
            wprintf(L"\n❌ 安裝失敗，執行回滾...\n");
            return false;  // 会触发回滚
        }
    } else {
        wprintf(L"  ✓ 成功\n");
        state.driverInstalled = true;
    }
    
    return true;
}

// 验证安装
void VerifyInstallation() {
    wprintf(L"\n========================================\n");
    wprintf(L"驗證安裝結果\n");
    wprintf(L"========================================\n\n");
    
    GUID displayClassGuid = { 0x4D36E968, 0xE325, 0x11CE, 
        { 0xBF, 0xC1, 0x08, 0x00, 0x2B, 0xE1, 0x03, 0x18 } };
    
    HDEVINFO deviceInfoSet = SetupDiGetClassDevsW(
        &displayClassGuid, NULL, NULL, DIGCF_PRESENT);
    
    if (deviceInfoSet != INVALID_HANDLE_VALUE) {
        SP_DEVINFO_DATA devInfoData = {};
        devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
        
        bool found = false;
        
        for (DWORD i = 0; SetupDiEnumDeviceInfo(deviceInfoSet, i, &devInfoData); i++) {
            wchar_t deviceId[MAX_PATH] = {};
            wchar_t deviceDesc[MAX_PATH] = {};
            
            if (SetupDiGetDeviceInstanceIdW(deviceInfoSet, &devInfoData, deviceId, MAX_PATH, NULL)) {
                SetupDiGetDeviceRegistryPropertyW(
                    deviceInfoSet, &devInfoData, SPDRP_DEVICEDESC,
                    NULL, (BYTE*)deviceDesc, sizeof(deviceDesc), NULL);
                
                if (wcsstr(deviceId, L"IddSampleDriver") != NULL ||
                    wcsstr(deviceDesc, L"IddSampleDriver") != NULL) {
                    
                    wprintf(L"✓ 找到設備:\n");
                    wprintf(L"  名稱: %s\n", deviceDesc);
                    wprintf(L"  ID: %s\n", deviceId);
                    
                    // 检查状态
                    ULONG problem = 0;
                    ULONG status = 0;
                    DEVINST devInst = 0;
                    
                    if (CM_Locate_DevNodeW(&devInst, deviceId, CM_LOCATE_DEVNODE_NORMAL) == CR_SUCCESS) {
                        if (CM_Get_DevNode_Status(&status, &problem, devInst, 0) == CR_SUCCESS) {
                            if (problem == 0) {
                                wprintf(L"  狀態: ✓ 正常工作\n");
                            } else {
                                wprintf(L"  狀態: ⚠ 有問題 (代碼: %d)\n", problem);
                            }
                        }
                    }
                    
                    found = true;
                    break;
                }
            }
        }
        
        SetupDiDestroyDeviceInfoList(deviceInfoSet);
        
        if (!found) {
            wprintf(L"✗ 未找到 IddSampleDriver 設備\n");
        }
    }
}

// 主函数
int wmain(int argc, wchar_t* argv[]) {
    wprintf(L"========================================\n");
    wprintf(L"SetupAPI 驅動安裝 (簡單方法 + 回滾)\n");
    wprintf(L"========================================\n\n");
    
    // 获取 INF 路径
    wchar_t infPath[MAX_PATH] = L"IddSampleDriver_Fixed.inf";
    if (argc > 1) {
        wcscpy_s(infPath, argv[1]);
    }
    
    // 获取完整路径
    wchar_t fullPath[MAX_PATH];
    if (!GetFullPathNameW(infPath, MAX_PATH, fullPath, NULL)) {
        wprintf(L"錯誤: 無法取得完整路徑\n");
        return 1;
    }
    
    // 检查文件是否存在
    if (GetFileAttributesW(fullPath) == INVALID_FILE_ATTRIBUTES) {
        wprintf(L"錯誤: 找不到 INF 文件: %s\n", fullPath);
        return 1;
    }
    
    wprintf(L"INF 文件: %s\n", fullPath);
    
    // 安装状态
    InstallState state;
    bool installSuccess = false;
    
    // 执行安装
    installSuccess = InstallDriver(fullPath, state);
    
    if (!installSuccess) {
        wprintf(L"\n❌ 安裝失敗！\n");
        
        // 自动回滚
        if (state.deviceRegistered) {
            RollbackInstallation(state);
        } else {
            // 清理资源
            if (state.deviceInfoSet != INVALID_HANDLE_VALUE) {
                SetupDiDestroyDeviceInfoList(state.deviceInfoSet);
            }
        }
        
        wprintf(L"\n請檢查錯誤信息\n");
        return 1;
    }
    
    // 成功
    wprintf(L"\n========================================\n");
    wprintf(L"✓ 安裝成功！\n");
    wprintf(L"========================================\n");
    
    // 清理资源
    if (state.deviceInfoSet != INVALID_HANDLE_VALUE) {
        SetupDiDestroyDeviceInfoList(state.deviceInfoSet);
    }
    
    // 验证安装
    VerifyInstallation();
    
    wprintf(L"\n請檢查設備管理器確認安裝結果\n");
    
    return 0;
}

