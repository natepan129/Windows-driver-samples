// SetupAPI 驅動程式卸載工具
#include <windows.h>
#include <setupapi.h>
#include <newdev.h>
#include <cfgmgr32.h>
#include <stdio.h>

#pragma comment(lib, "setupapi.lib")
#pragma comment(lib, "cfgmgr32.lib")
#pragma comment(lib, "advapi32.lib")

// 卸載特定設備
bool UninstallDevice(HDEVINFO deviceInfoSet, PSP_DEVINFO_DATA devInfoData, const wchar_t* deviceName) {
    wprintf(L"  正在卸載: %s\n", deviceName);
    
    // 準備移除參數
    SP_REMOVEDEVICE_PARAMS removeParams = {};
    removeParams.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
    removeParams.ClassInstallHeader.InstallFunction = DIF_REMOVE;
    removeParams.Scope = DI_REMOVEDEVICE_GLOBAL;  // 全局移除
    removeParams.HwProfile = 0;
    
    // 設置類安裝參數
    if (!SetupDiSetClassInstallParamsW(
            deviceInfoSet,
            devInfoData,
            (PSP_CLASSINSTALL_HEADER)&removeParams,
            sizeof(removeParams))) {
        wprintf(L"    失敗: SetupDiSetClassInstallParams (錯誤: %d)\n", GetLastError());
        return false;
    }
    
    // 調用類安裝器進行移除
    if (!SetupDiCallClassInstaller(DIF_REMOVE, deviceInfoSet, devInfoData)) {
        wprintf(L"    失敗: DIF_REMOVE (錯誤: %d)\n", GetLastError());
        return false;
    }
    
    wprintf(L"    成功\n");
    return true;
}

// 主函數
int wmain(int argc, wchar_t* argv[]) {
    wprintf(L"========================================\n");
    wprintf(L"SetupAPI 驅動程式卸載工具\n");
    wprintf(L"========================================\n\n");
    
    bool foundDevice = false;
    int removedCount = 0;
    
    // 顯示適配器類別 GUID
    GUID displayClassGuid = { 0x4D36E968, 0xE325, 0x11CE, 
        { 0xBF, 0xC1, 0x08, 0x00, 0x2B, 0xE1, 0x03, 0x18 } };
    
    // ========================================
    // 步驟 1: 枚舉並移除所有 IddSampleDriver 設備
    // ========================================
    
    wprintf(L"[步驟 1] 搜尋 IddSampleDriver 設備...\n");
    wprintf(L"----------------------------------------\n");
    
    // 獲取顯示設備列表
    HDEVINFO deviceInfoSet = SetupDiGetClassDevsW(
        &displayClassGuid,
        NULL,
        NULL,
        DIGCF_PRESENT | DIGCF_ALLCLASSES
    );
    
    if (deviceInfoSet == INVALID_HANDLE_VALUE) {
        wprintf(L"錯誤: 無法獲取設備列表 (錯誤: %d)\n", GetLastError());
        return 1;
    }
    
    SP_DEVINFO_DATA devInfoData = {};
    devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
    
    // 枚舉所有設備
    for (DWORD i = 0; SetupDiEnumDeviceInfo(deviceInfoSet, i, &devInfoData); i++) {
        wchar_t deviceId[MAX_PATH] = {};
        wchar_t deviceDesc[MAX_PATH] = {};
        bool isOurDevice = false;
        
        // 獲取設備實例 ID
        if (SetupDiGetDeviceInstanceIdW(deviceInfoSet, &devInfoData, deviceId, MAX_PATH, NULL)) {
            // 獲取設備描述
            SetupDiGetDeviceRegistryPropertyW(
                deviceInfoSet,
                &devInfoData,
                SPDRP_DEVICEDESC,
                NULL,
                (BYTE*)deviceDesc,
                sizeof(deviceDesc),
                NULL);
            
            // 檢查是否是我們的驅動（檢查實例 ID 或設備描述）
            if (wcsstr(deviceId, L"IddSampleDriver") != NULL ||
                wcsstr(deviceId, L"IDDSAMPLEDRIVER") != NULL ||
                wcsstr(deviceDesc, L"IddSampleDriver") != NULL ||
                wcsstr(deviceDesc, L"IDDSAMPLEDRIVER") != NULL) {
                
                isOurDevice = true;
                foundDevice = true;
                
                if (wcslen(deviceDesc) > 0) {
                    wprintf(L"找到設備: %s\n", deviceDesc);
                } else {
                    wprintf(L"找到設備: (無描述)\n");
                }
                
                wprintf(L"  實例 ID: %s\n", deviceId);
                
                // 卸載設備
                if (UninstallDevice(deviceInfoSet, &devInfoData, deviceDesc)) {
                    removedCount++;
                }
            }
        }
    }
    
    SetupDiDestroyDeviceInfoList(deviceInfoSet);
    
    if (!foundDevice) {
        wprintf(L"未找到 IddSampleDriver 設備\n");
    } else {
        wprintf(L"\n移除了 %d 個設備\n", removedCount);
    }
    
    // ========================================
    // 步驟 2: 從驅動程式存儲區移除驅動
    // ========================================
    
    wprintf(L"\n[步驟 2] 從驅動程式存儲區移除...\n");
    wprintf(L"----------------------------------------\n");
    
    // 使用 pnputil 來列出和移除驅動
    wchar_t cmd[1024];
    swprintf_s(cmd, L"pnputil /enum-drivers | findstr /i \"iddsampledriver\" > drivers_found.txt");
    _wsystem(cmd);
    
    FILE* fp = NULL;
    if (_wfopen_s(&fp, L"drivers_found.txt", L"r") == 0 && fp != NULL) {
        char line[512];
        bool foundDriver = false;
        
        while (fgets(line, sizeof(line), fp)) {
            // 查找 oem*.inf
            char* oemPtr = strstr(line, "oem");
            if (oemPtr) {
                char oemFile[50] = {};
                sscanf_s(oemPtr, "%s", oemFile, (unsigned)sizeof(oemFile));
                
                // 移除 .inf 後綴以獲得 OEM 編號
                char* dotPtr = strstr(oemFile, ".inf");
                if (dotPtr) *dotPtr = '\0';
                
                wprintf(L"  找到驅動: %S\n", oemFile);
                
                // 使用 pnputil 刪除
                swprintf_s(cmd, L"pnputil /delete-driver %S /uninstall /force", oemFile);
                wprintf(L"  執行: %s\n", cmd);
                
                int result = _wsystem(cmd);
                if (result == 0) {
                    wprintf(L"  成功移除驅動\n");
                    foundDriver = true;
                } else {
                    wprintf(L"  警告: 移除失敗 (返回碼: %d)\n", result);
                }
            }
        }
        
        fclose(fp);
        DeleteFileW(L"drivers_found.txt");
        
        if (!foundDriver) {
            wprintf(L"  未在驅動程式存儲區中找到 IddSampleDriver\n");
        }
    } else {
        wprintf(L"  無法檢查驅動程式存儲區\n");
    }
    
    // ========================================
    // 步驟 3: 清理註冊表
    // ========================================
    
    wprintf(L"\n[步驟 3] 清理註冊表項目...\n");
    wprintf(L"----------------------------------------\n");
    
    const wchar_t* regPaths[] = {
        L"SYSTEM\\CurrentControlSet\\Enum\\ROOT\\IddSampleDriver",
        L"SYSTEM\\CurrentControlSet\\Enum\\ROOT\\IDDSAMPLEDRIVER"
    };
    
    for (int i = 0; i < sizeof(regPaths) / sizeof(regPaths[0]); i++) {
        HKEY hKey;
        LONG result = RegOpenKeyExW(HKEY_LOCAL_MACHINE, regPaths[i], 0, KEY_READ, &hKey);
        
        if (result == ERROR_SUCCESS) {
            RegCloseKey(hKey);
            
            wprintf(L"  刪除: %s\n", regPaths[i]);
            
            // 使用 reg delete 命令刪除（需要管理員權限）
            wchar_t regCmd[512];
            swprintf_s(regCmd, L"reg delete \"HKLM\\%s\" /f > nul 2>&1", regPaths[i]);
            
            if (_wsystem(regCmd) == 0) {
                wprintf(L"    成功\n");
            } else {
                wprintf(L"    警告: 可能未完全刪除\n");
            }
        }
    }
    
    // ========================================
    // 步驟 4: 驗證卸載
    // ========================================
    
    wprintf(L"\n[步驟 4] 驗證卸載...\n");
    wprintf(L"----------------------------------------\n");
    
    // 重新掃描硬體
    wprintf(L"  觸發硬體重新掃描...\n");
    _wsystem(L"pnputil /scan-devices > nul 2>&1");
    
    Sleep(1000);  // 等待系統更新
    
    // 再次檢查設備
    deviceInfoSet = SetupDiGetClassDevsW(
        &displayClassGuid,
        NULL,
        NULL,
        DIGCF_PRESENT | DIGCF_ALLCLASSES
    );
    
    if (deviceInfoSet != INVALID_HANDLE_VALUE) {
        bool stillExists = false;
        devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
        
        for (DWORD i = 0; SetupDiEnumDeviceInfo(deviceInfoSet, i, &devInfoData); i++) {
            wchar_t deviceId[MAX_PATH] = {};
            
            if (SetupDiGetDeviceInstanceIdW(deviceInfoSet, &devInfoData, deviceId, MAX_PATH, NULL)) {
                if (wcsstr(deviceId, L"IddSampleDriver") != NULL ||
                    wcsstr(deviceId, L"IDDSAMPLEDRIVER") != NULL) {
                    wprintf(L"  警告: 仍然存在設備: %s\n", deviceId);
                    stillExists = true;
                }
            }
        }
        
        SetupDiDestroyDeviceInfoList(deviceInfoSet);
        
        if (!stillExists) {
            wprintf(L"  ✓ 所有設備已成功移除\n");
        }
    }
    
    wprintf(L"\n========================================\n");
    wprintf(L"卸載完成\n");
    wprintf(L"========================================\n");
    wprintf(L"\n建議重新啟動系統以完全清理\n");
    
    return 0;
}

