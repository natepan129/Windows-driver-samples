// 帶回滾機制的 SetupAPI 驅動程式安裝工具
#include <windows.h>
#include <setupapi.h>
#include <newdev.h>
#include <cfgmgr32.h>
#include <stdio.h>
#include <vector>
#include <string>

#pragma comment(lib, "setupapi.lib")
#pragma comment(lib, "newdev.lib")
#pragma comment(lib, "cfgmgr32.lib")

// 安裝狀態記錄
struct InstallState {
    bool deviceCreated;
    bool deviceRegistered;
    bool driverInstalled;
    std::wstring deviceInstanceId;
    HDEVINFO deviceInfoSet;
    SP_DEVINFO_DATA devInfoData;
    
    InstallState() : deviceCreated(false), deviceRegistered(false), 
                     driverInstalled(false), deviceInfoSet(INVALID_HANDLE_VALUE) {
        devInfoData = {};
    }
};

// 保存安裝前的狀態
bool SavePreInstallState(const wchar_t* filename) {
    wprintf(L"[備份] 保存安裝前狀態...\n");
    
    FILE* fp = NULL;
    if (_wfopen_s(&fp, filename, L"w") != 0 || fp == NULL) {
        wprintf(L"  警告: 無法創建備份文件\n");
        return false;
    }
    
    // 記錄時間戳
    SYSTEMTIME st;
    GetLocalTime(&st);
    fwprintf(fp, L"[InstallState]\n");
    fwprintf(fp, L"Timestamp=%04d-%02d-%02d %02d:%02d:%02d\n",
             st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
    
    // 列出現有的顯示設備
    GUID displayClassGuid = { 0x4D36E968, 0xE325, 0x11CE, 
        { 0xBF, 0xC1, 0x08, 0x00, 0x2B, 0xE1, 0x03, 0x18 } };
    
    HDEVINFO deviceInfoSet = SetupDiGetClassDevsW(
        &displayClassGuid, NULL, NULL, DIGCF_PRESENT);
    
    if (deviceInfoSet != INVALID_HANDLE_VALUE) {
        SP_DEVINFO_DATA devInfoData = {};
        devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
        
        fwprintf(fp, L"\n[ExistingDevices]\n");
        int count = 0;
        
        for (DWORD i = 0; SetupDiEnumDeviceInfo(deviceInfoSet, i, &devInfoData); i++) {
            wchar_t deviceId[MAX_PATH] = {};
            wchar_t deviceDesc[MAX_PATH] = {};
            
            if (SetupDiGetDeviceInstanceIdW(deviceInfoSet, &devInfoData, deviceId, MAX_PATH, NULL)) {
                SetupDiGetDeviceRegistryPropertyW(
                    deviceInfoSet, &devInfoData, SPDRP_DEVICEDESC,
                    NULL, (BYTE*)deviceDesc, sizeof(deviceDesc), NULL);
                
                fwprintf(fp, L"Device%d_ID=%s\n", count, deviceId);
                fwprintf(fp, L"Device%d_Desc=%s\n", count, deviceDesc);
                count++;
            }
        }
        
        fwprintf(fp, L"DeviceCount=%d\n", count);
        SetupDiDestroyDeviceInfoList(deviceInfoSet);
    }
    
    fclose(fp);
    wprintf(L"  ✓ 狀態已保存到: %s\n", filename);
    return true;
}

// 回滾操作
bool RollbackInstallation(InstallState& state) {
    wprintf(L"\n========================================\n");
    wprintf(L"執行回滾操作...\n");
    wprintf(L"========================================\n\n");
    
    bool success = true;
    
    // 如果驅動已安裝，移除設備
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
    
    // 清理設備信息集
    if (state.deviceInfoSet != INVALID_HANDLE_VALUE) {
        SetupDiDestroyDeviceInfoList(state.deviceInfoSet);
        state.deviceInfoSet = INVALID_HANDLE_VALUE;
    }
    
    // 清理註冊表（如果需要）
    if (!state.deviceInstanceId.empty()) {
        wprintf(L"[回滾] 清理註冊表項目...\n");
        wchar_t regCmd[512];
        swprintf_s(regCmd, L"reg delete \"HKLM\\SYSTEM\\CurrentControlSet\\Enum\\%s\" /f > nul 2>&1",
                   state.deviceInstanceId.c_str());
        _wsystem(regCmd);
        wprintf(L"  完成\n");
    }
    
    // 重置狀態
    state.deviceCreated = false;
    state.deviceRegistered = false;
    state.driverInstalled = false;
    
    if (success) {
        wprintf(L"\n✓ 回滾成功\n");
    } else {
        wprintf(L"\n✗ 回滾過程中出現錯誤，可能需要手動清理\n");
    }
    
    return success;
}

// 主安裝函數
bool InstallDriver(const wchar_t* infPath, InstallState& state) {
    wprintf(L"\n========================================\n");
    wprintf(L"開始安裝驅動程式\n");
    wprintf(L"========================================\n\n");
    
    wprintf(L"INF 文件: %s\n\n", infPath);
    
    // 顯示適配器類別 GUID
    GUID displayClassGuid = { 0x4D36E968, 0xE325, 0x11CE, 
        { 0xBF, 0xC1, 0x08, 0x00, 0x2B, 0xE1, 0x03, 0x18 } };
    
    // 步驟 1: 創建設備信息列表
    wprintf(L"[1/5] 創建設備信息列表...\n");
    state.deviceInfoSet = SetupDiCreateDeviceInfoList(&displayClassGuid, NULL);
    
    if (state.deviceInfoSet == INVALID_HANDLE_VALUE) {
        wprintf(L"  ✗ 失敗 (錯誤: %d)\n", GetLastError());
        return false;
    }
    wprintf(L"  ✓ 成功\n");
    
    // 步驟 2: 創建設備信息
    wprintf(L"[2/5] 創建設備信息...\n");
    
    state.devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
    
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
    
    // 步驟 3: 設置硬體 ID
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
    
    // 步驟 4: 註冊設備
    wprintf(L"[4/5] 註冊設備...\n");
    
    if (!SetupDiCallClassInstaller(DIF_REGISTERDEVICE, state.deviceInfoSet, &state.devInfoData)) {
        wprintf(L"  ✗ 失敗 (錯誤: %d)\n", GetLastError());
        return false;
    }
    
    state.deviceRegistered = true;
    
    // 獲取設備實例 ID
    wchar_t deviceId[MAX_PATH] = {};
    if (SetupDiGetDeviceInstanceIdW(state.deviceInfoSet, &state.devInfoData, deviceId, MAX_PATH, NULL)) {
        state.deviceInstanceId = deviceId;
        wprintf(L"  ✓ 成功 (ID: %s)\n", deviceId);
    } else {
        wprintf(L"  ✓ 成功\n");
    }
    
    // 步驟 5: 設置驅動路徑並安裝
    wprintf(L"[5/5] 安裝驅動程式...\n");
    
    // 關鍵修復：設置 INF 路徑，讓 DIF_INSTALLDEVICE 知道使用哪個 INF
    SP_DEVINSTALL_PARAMS_W deviceInstallParams = {};
    deviceInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS_W);
    
    if (SetupDiGetDeviceInstallParamsW(state.deviceInfoSet, &state.devInfoData, &deviceInstallParams)) {
        // 設置 INF 路徑
        wcsncpy_s(deviceInstallParams.DriverPath, MAX_PATH, infPath, _TRUNCATE);
        deviceInstallParams.Flags |= DI_ENUMSINGLEINF;  // 只使用指定的 INF
        
        if (SetupDiSetDeviceInstallParamsW(state.deviceInfoSet, &state.devInfoData, &deviceInstallParams)) {
            wprintf(L"  INF 路徑已設置\n");
            
            // 構建驅動列表（關鍵步驟！）
            if (SetupDiBuildDriverInfoList(state.deviceInfoSet, &state.devInfoData, SPDIT_CLASSDRIVER)) {
                wprintf(L"  驅動列表已構建\n");
                
                // 選擇最佳驅動
                if (SetupDiCallClassInstaller(DIF_SELECTBESTCOMPATDRV, state.deviceInfoSet, &state.devInfoData)) {
                    wprintf(L"  已選擇驅動\n");
                }
            }
        }
    }
    
    // 現在調用 DIF_INSTALLDEVICE 應該會成功
    if (!SetupDiCallClassInstaller(DIF_INSTALLDEVICE, state.deviceInfoSet, &state.devInfoData)) {
        DWORD error = GetLastError();
        wprintf(L"  ✗ DIF_INSTALLDEVICE 失敗 (錯誤: %d)\n", error);
        
        // 嘗試備用方法
        wprintf(L"  嘗試使用 UpdateDriverForPlugAndPlayDevices...\n");
        
        BOOL reboot = FALSE;
        if (UpdateDriverForPlugAndPlayDevicesW(
                NULL,
                L"ROOT\\IddSampleDriver",
                infPath,
                INSTALLFLAG_FORCE,
                &reboot)) {
            wprintf(L"  ✓ 成功 (使用備用方法)\n");
            state.driverInstalled = true;
            
            if (reboot) {
                wprintf(L"  ⚠ 需要重新啟動\n");
            }
        } else {
            wprintf(L"  ✗ 備用方法也失敗 (錯誤: %d)\n", GetLastError());
            return false;
        }
    } else {
        wprintf(L"  ✓ 成功\n");
        state.driverInstalled = true;
    }
    
    return true;
}

// 驗證安裝
void VerifyInstallation() {
    wprintf(L"\n========================================\n");
    wprintf(L"驗證安裝\n");
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
                if (wcsstr(deviceId, L"IddSampleDriver") != NULL ||
                    wcsstr(deviceId, L"IDDSAMPLEDRIVER") != NULL) {
                    
                    SetupDiGetDeviceRegistryPropertyW(
                        deviceInfoSet, &devInfoData, SPDRP_DEVICEDESC,
                        NULL, (BYTE*)deviceDesc, sizeof(deviceDesc), NULL);
                    
                    wprintf(L"✓ 找到設備: %s\n", deviceDesc);
                    wprintf(L"  實例 ID: %s\n", deviceId);
                    
                    // 檢查設備狀態
                    ULONG status = 0, problem = 0;
                    if (CM_Get_DevNode_Status(&status, &problem, devInfoData.DevInst, 0) == CR_SUCCESS) {
                        if (problem == 0) {
                            wprintf(L"  狀態: 正常工作\n");
                        } else {
                            wprintf(L"  狀態: 有問題 (代碼: %d)\n", problem);
                        }
                    }
                    
                    found = true;
                }
            }
        }
        
        SetupDiDestroyDeviceInfoList(deviceInfoSet);
        
        if (!found) {
            wprintf(L"✗ 未找到 IddSampleDriver 設備\n");
        }
    }
}

// 主函數
int wmain(int argc, wchar_t* argv[]) {
    wprintf(L"========================================\n");
    wprintf(L"SetupAPI 驅動程式安裝工具 (帶回滾)\n");
    wprintf(L"========================================\n\n");
    
    // 取得 INF 路徑
    wchar_t infPath[MAX_PATH] = L"IddSampleDriver_Fixed.inf";
    if (argc > 1) {
        wcscpy_s(infPath, argv[1]);
    }
    
    // 取得完整路徑
    wchar_t fullPath[MAX_PATH];
    if (!GetFullPathNameW(infPath, MAX_PATH, fullPath, NULL)) {
        wprintf(L"錯誤: 無法取得完整路徑\n");
        return 1;
    }
    
    // 檢查文件是否存在
    if (GetFileAttributesW(fullPath) == INVALID_FILE_ATTRIBUTES) {
        wprintf(L"錯誤: 找不到 INF 文件: %s\n", fullPath);
        return 1;
    }
    
    // 保存安裝前狀態
    const wchar_t* backupFile = L"driver_install_backup.txt";
    SavePreInstallState(backupFile);
    wprintf(L"\n");
    
    // 安裝狀態
    InstallState state;
    bool installSuccess = false;
    
    // 執行安裝
    installSuccess = InstallDriver(fullPath, state);
    
    if (!installSuccess) {
        wprintf(L"\n❌ 安裝失敗！\n");
        
        // 詢問是否回滾
        wprintf(L"\n是否要回滾更改? (Y/N): ");
        wchar_t response[10];
        if (fgetws(response, 10, stdin) && (response[0] == L'Y' || response[0] == L'y')) {
            RollbackInstallation(state);
        } else {
            wprintf(L"跳過回滾。請手動清理或稍後運行 uninstall_driver.exe\n");
            
            // 清理設備信息集
            if (state.deviceInfoSet != INVALID_HANDLE_VALUE) {
                SetupDiDestroyDeviceInfoList(state.deviceInfoSet);
            }
        }
        
        return 1;
    }
    
    // 驗證安裝
    VerifyInstallation();
    
    // 清理設備信息集
    if (state.deviceInfoSet != INVALID_HANDLE_VALUE) {
        SetupDiDestroyDeviceInfoList(state.deviceInfoSet);
    }
    
    wprintf(L"\n========================================\n");
    wprintf(L"✓ 安裝成功完成\n");
    wprintf(L"========================================\n");
    wprintf(L"\n備份文件: %s\n", backupFile);
    wprintf(L"如需卸載，請運行: uninstall_driver.exe\n");
    
    return 0;
}

