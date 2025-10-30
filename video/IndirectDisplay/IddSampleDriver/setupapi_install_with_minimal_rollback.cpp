// 完全基于 setupapi_install_simple.cpp（已验证成功）
// 只添加最小的 rollback 逻辑
#include <windows.h>
#include <setupapi.h>
#include <newdev.h>
#include <stdio.h>

#pragma comment(lib, "setupapi.lib")
#pragma comment(lib, "newdev.lib")

int wmain(int argc, wchar_t* argv[]) {
    wprintf(L"========================================\n");
    wprintf(L"SetupAPI 驅動程式安裝工具 (帶自動回滾)\n");
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
    
    wprintf(L"INF 文件: %s\n\n", fullPath);
    
    wprintf(L"使用 SetupDiCreateDeviceInfo 創建設備...\n");
    wprintf(L"----------------------------------------\n");
    
    // 顯示適配器類別 GUID
    GUID displayClassGuid = { 0x4D36E968, 0xE325, 0x11CE, 
        { 0xBF, 0xC1, 0x08, 0x00, 0x2B, 0xE1, 0x03, 0x18 } };
    
    // 步驟 1: 創建設備信息列表
    wprintf(L"[1] 創建設備信息列表...\n");
    HDEVINFO deviceInfoSet = SetupDiCreateDeviceInfoList(&displayClassGuid, NULL);
    
    if (deviceInfoSet == INVALID_HANDLE_VALUE) {
        wprintf(L"失敗: SetupDiCreateDeviceInfoList (錯誤: %d)\n", GetLastError());
        return 1;
    }
    wprintf(L"成功\n");
    
    // 步驟 2: 創建設備信息
    wprintf(L"[2] 創建設備信息...\n");
    
    SP_DEVINFO_DATA devInfoData = {};
    devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
    
    if (!SetupDiCreateDeviceInfoW(
            deviceInfoSet,
            L"IddSampleDriver",
            &displayClassGuid,
            L"IddSampleDriver Device",
            NULL,
            DICD_GENERATE_ID,
            &devInfoData)) {
        wprintf(L"失敗: SetupDiCreateDeviceInfo (錯誤: %d)\n", GetLastError());
        SetupDiDestroyDeviceInfoList(deviceInfoSet);
        return 1;
    }
    wprintf(L"成功\n");
    
    // 步驟 3: 設置硬體 ID
    wprintf(L"[3] 設置硬體 ID...\n");
    
    wchar_t hardwareId[] = L"ROOT\\IddSampleDriver\0\0";
    
    if (!SetupDiSetDeviceRegistryPropertyW(
            deviceInfoSet,
            &devInfoData,
            SPDRP_HARDWAREID,
            (BYTE*)hardwareId,
            sizeof(hardwareId))) {
        wprintf(L"失敗: SetupDiSetDeviceRegistryProperty (錯誤: %d)\n", GetLastError());
        SetupDiDestroyDeviceInfoList(deviceInfoSet);
        return 1;
    }
    wprintf(L"成功\n");
    
    // 步驟 4: 註冊設備（關鍵點！）
    wprintf(L"[4] 註冊設備...\n");
    
    if (!SetupDiCallClassInstaller(DIF_REGISTERDEVICE, deviceInfoSet, &devInfoData)) {
        wprintf(L"失敗: DIF_REGISTERDEVICE (錯誤: %d)\n", GetLastError());
        SetupDiDestroyDeviceInfoList(deviceInfoSet);
        return 1;
    }
    wprintf(L"成功\n");
    
    // ⭐ 新增：標記設備已註冊（回滾關鍵點）
    bool deviceRegistered = true;
    bool installSuccess = false;
    
    // 步驟 5: 安裝驅動程式
    wprintf(L"[5] 安裝驅動程式...\n");
    
    if (!SetupDiCallClassInstaller(DIF_INSTALLDEVICE, deviceInfoSet, &devInfoData)) {
        DWORD error = GetLastError();
        wprintf(L"失敗: DIF_INSTALLDEVICE (錯誤: %d)\n", error);
        
        // 即使失敗，也嘗試用其他方法
        wprintf(L"\n嘗試使用 UpdateDriverForPlugAndPlayDevices...\n");
        
        BOOL reboot = FALSE;
        if (UpdateDriverForPlugAndPlayDevicesW(
                NULL,
                L"ROOT\\IddSampleDriver",
                fullPath,
                INSTALLFLAG_FORCE,
                &reboot)) {
            wprintf(L"成功通過 UpdateDriverForPlugAndPlayDevices 安裝\n");
            if (reboot) {
                wprintf(L"警告: 需要重新啟動\n");
            }
            installSuccess = true;  // ⭐ 標記成功
        } else {
            wprintf(L"UpdateDriverForPlugAndPlayDevices 也失敗了 (錯誤: %d)\n", GetLastError());
            // ⭐ installSuccess 保持 false，會觸發回滾
        }
    } else {
        wprintf(L"成功!\n");
        installSuccess = true;  // ⭐ 標記成功
    }
    
    // ⭐ 新增：回滾邏輯
    if (!installSuccess && deviceRegistered) {
        wprintf(L"\n========================================\n");
        wprintf(L"⚠ 安裝失敗，執行自動回滾...\n");
        wprintf(L"========================================\n");
        
        SP_REMOVEDEVICE_PARAMS removeParams = {};
        removeParams.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
        removeParams.ClassInstallHeader.InstallFunction = DIF_REMOVE;
        removeParams.Scope = DI_REMOVEDEVICE_GLOBAL;
        removeParams.HwProfile = 0;
        
        if (SetupDiSetClassInstallParamsW(
                deviceInfoSet,
                &devInfoData,
                (PSP_CLASSINSTALL_HEADER)&removeParams,
                sizeof(removeParams))) {
            
            if (SetupDiCallClassInstaller(DIF_REMOVE, deviceInfoSet, &devInfoData)) {
                wprintf(L"✓ 設備已回滾移除\n");
            } else {
                wprintf(L"✗ 回滾失敗 (錯誤: %d) - 可能需要手動清理\n", GetLastError());
            }
        }
    }
    
    // 清理
    SetupDiDestroyDeviceInfoList(deviceInfoSet);
    
    if (installSuccess) {
        wprintf(L"\n========================================\n");
        wprintf(L"✓ 安裝成功！\n");
        wprintf(L"========================================\n");
        wprintf(L"\n請檢查設備管理器以確認安裝結果\n");
        return 0;
    } else {
        wprintf(L"\n========================================\n");
        wprintf(L"✗ 安裝失敗\n");
        wprintf(L"========================================\n");
        return 1;
    }
}

