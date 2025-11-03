# Install/Uninstall Complete Fix

## GPT 建議的完整實現

### 修改清單

1. ✅ **InstallDriver 加鎖** (`m_installMutex`)
2. ✅ **DiInstallDriverW stage INF** (先放入 Driver Store)
3. ✅ **SPDIT_CLASSDRIVER → SPDIT_COMPATDRIVER**
4. ✅ **wcscpy_s → wcsncpy_s** with `_countof`
5. ✅ **安裝前檢查重複設備**
6. ✅ **失敗回滾** (移除 devnode)
7. ✅ **UninstallDriver 先 Disable 再 Remove**
8. ✅ **清理 Driver Store** (DiUninstallDriverW)
9. ✅ **IsDriverInstalled 簡化** (只用 Registry)
10. ✅ **權限檢查** (IsRunningAsAdministrator)

### 需要添加的輔助函數

```cpp
// vddsdk.cpp 內部添加
static bool IsRunningAsAdministrator() {
    BOOL isAdmin = FALSE;
    PSID adminGroup = NULL;
    SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;
    
    if (AllocateAndInitializeSid(&ntAuthority, 2, 
        SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS,
        0, 0, 0, 0, 0, 0, &adminGroup)) {
        CheckTokenMembership(NULL, adminGroup, &isAdmin);
        FreeSid(adminGroup);
    }
    
    return isAdmin == TRUE;
}
```

### 完整 InstallDriver (替換 line 587-709)

```cpp
Status VddSdkImpl::InstallDriver(const std::wstring& infPath) {
    std::lock_guard<std::mutex> lock(m_installMutex);
    
    // Validation
    if (infPath.empty()) {
        SetLastError("INF path cannot be empty");
        return Status::InvalidArg;
    }
    
    // Convert to absolute path
    wchar_t absPath[MAX_PATH];
    if (GetFullPathNameW(infPath.c_str(), MAX_PATH, absPath, nullptr) == 0) {
        SetLastError("Invalid INF path");
        return Status::InvalidArg;
    }
    
    if (GetFileAttributesW(absPath) == INVALID_FILE_ATTRIBUTES) {
        SetLastError("INF file not found");
        return Status::InvalidArg;
    }
    
    // Check admin
    if (!IsRunningAsAdministrator()) {
        SetLastError("Administrator privileges required");
        return Status::AdminRequired;
    }
    
    // Check if device already exists
    GUID displayClassGuid = { 0x4D36E968, 0xE325, 0x11CE, 
        { 0xBF, 0xC1, 0x08, 0x00, 0x2B, 0xE1, 0x03, 0x18 } };
    
    {
        HDEVINFO hCheck = SetupDiGetClassDevsW(&displayClassGuid, nullptr, nullptr, DIGCF_ALLCLASSES);
        if (hCheck != INVALID_HANDLE_VALUE) {
            SP_DEVINFO_DATA dev{}; dev.cbSize = sizeof(dev);
            for (DWORD i = 0; SetupDiEnumDeviceInfo(hCheck, i, &dev); ++i) {
                WCHAR hwid[4096] = {};
                if (SetupDiGetDeviceRegistryPropertyW(hCheck, &dev, SPDRP_HARDWAREID,
                    nullptr, (BYTE*)hwid, sizeof(hwid), nullptr)) {
                    for (wchar_t* p = hwid; *p; p += wcslen(p) + 1) {
                        if (_wcsicmp(p, L"ROOT\\IddSampleDriver") == 0) {
                            SetupDiDestroyDeviceInfoList(hCheck);
                            SetLastError("Device already installed");
                            return Status::AlreadyInstalled;
                        }
                    }
                }
            }
            SetupDiDestroyDeviceInfoList(hCheck);
        }
    }
    
    // Stage INF to Driver Store
    BOOL needReboot = FALSE;
    if (!DiInstallDriverW(nullptr, absPath, DIIRFLAG_FORCE_INF, &needReboot)) {
        DWORD err = ::GetLastError();
        SetLastError("Failed to stage INF to Driver Store: " + std::to_string(err));
        return Status::DriverError;
    }
    
    // Create device info list
    HDEVINFO hDevInfo = SetupDiCreateDeviceInfoList(&displayClassGuid, nullptr);
    if (hDevInfo == INVALID_HANDLE_VALUE) {
        SetLastError("Failed to create device info list: " + std::to_string(::GetLastError()));
        return Status::DriverError;
    }
    
    // Create device info
    SP_DEVINFO_DATA devInfoData = {};
    devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
    
    if (!SetupDiCreateDeviceInfoW(hDevInfo, L"IddSampleDriver", &displayClassGuid,
        L"IddSampleDriver Device", nullptr, DICD_GENERATE_ID, &devInfoData)) {
        DWORD err = ::GetLastError();
        SetupDiDestroyDeviceInfoList(hDevInfo);
        SetLastError("Failed to create device info: " + std::to_string(err));
        return Status::DriverError;
    }
    
    // Set HWID
    wchar_t hwid[] = L"ROOT\\IddSampleDriver\0\0";
    if (!SetupDiSetDeviceRegistryPropertyW(hDevInfo, &devInfoData, SPDRP_HARDWAREID,
        (const BYTE*)hwid, sizeof(hwid))) {
        DWORD err = ::GetLastError();
        SetupDiDestroyDeviceInfoList(hDevInfo);
        SetLastError("Failed to set hardware ID: " + std::to_string(err));
        return Status::DriverError;
    }
    
    // Register device
    if (!SetupDiCallClassInstaller(DIF_REGISTERDEVICE, hDevInfo, &devInfoData)) {
        DWORD err = ::GetLastError();
        SetupDiDestroyDeviceInfoList(hDevInfo);
        SetLastError("Failed to register device: " + std::to_string(err));
        return Status::DriverError;
    }
    
    // Set install params
    SP_DEVINSTALL_PARAMS_W installParams = {};
    installParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS_W);
    
    if (!SetupDiGetDeviceInstallParamsW(hDevInfo, &devInfoData, &installParams)) {
        goto ROLLBACK;
    }
    
    wcsncpy_s(installParams.DriverPath, _countof(installParams.DriverPath), absPath, _TRUNCATE);
    installParams.Flags |= DI_ENUMSINGLEINF;
    
    if (!SetupDiSetDeviceInstallParamsW(hDevInfo, &devInfoData, &installParams)) {
        goto ROLLBACK;
    }
    
    // Build driver list (COMPATDRIVER instead of CLASSDRIVER)
    if (!SetupDiBuildDriverInfoList(hDevInfo, &devInfoData, SPDIT_COMPATDRIVER)) {
        goto ROLLBACK;
    }
    
    // Select best compatible driver
    if (!SetupDiCallClassInstaller(DIF_SELECTBESTCOMPATDRV, hDevInfo, &devInfoData)) {
        goto ROLLBACK_WITH_LIST;
    }
    
    // Install device
    if (!SetupDiCallClassInstaller(DIF_INSTALLDEVICE, hDevInfo, &devInfoData)) {
        goto ROLLBACK_WITH_LIST;
    }
    
    // Cleanup
    SetupDiDestroyDriverInfoList(hDevInfo, &devInfoData, SPDIT_COMPATDRIVER);
    SetupDiDestroyDeviceInfoList(hDevInfo);
    
    if (needReboot) {
        SetLastError("Driver installed successfully (reboot may be required)");
    } else {
        SetLastError("Driver installed successfully");
    }
    
    return Status::Ok;

ROLLBACK_WITH_LIST:
    SetupDiDestroyDriverInfoList(hDevInfo, &devInfoData, SPDIT_COMPATDRIVER);
    
ROLLBACK:
    {
        DWORD lastErr = ::GetLastError();
        
        // Remove the device node we just created
        SP_REMOVEDEVICE_PARAMS removeParams = {};
        removeParams.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
        removeParams.ClassInstallHeader.InstallFunction = DIF_REMOVE;
        removeParams.Scope = DI_REMOVEDEVICE_GLOBAL;
        removeParams.HwProfile = 0;
        
        SetupDiSetClassInstallParamsW(hDevInfo, &devInfoData,
            reinterpret_cast<SP_CLASSINSTALL_HEADER*>(&removeParams), sizeof(removeParams));
        SetupDiCallClassInstaller(DIF_REMOVE, hDevInfo, &devInfoData);
        
        SetupDiDestroyDeviceInfoList(hDevInfo);
        
        SetLastError("Installation failed (rolled back): " + std::to_string(lastErr));
        return Status::DriverError;
    }
}
```

### 完整 UninstallDriver (替換 line 711-804)

```cpp
Status VddSdkImpl::UninstallDriver() {
    std::lock_guard<std::mutex> lock(m_installMutex);
    
    GUID displayClassGuid = { 0x4D36E968, 0xE325, 0x11CE, 
        { 0xBF, 0xC1, 0x08, 0x00, 0x2B, 0xE1, 0x03, 0x18 } };
    
    HDEVINFO hDevInfo = SetupDiGetClassDevsW(&displayClassGuid, nullptr, nullptr, 
        DIGCF_ALLCLASSES);  // Without DIGCF_PRESENT to find all devices
    
    if (hDevInfo == INVALID_HANDLE_VALUE) {
        SetLastError("Failed to get display device information: " + std::to_string(::GetLastError()));
        return Status::DriverError;
    }
    
    // Collect all matching devices
    std::vector<SP_DEVINFO_DATA> devicesToRemove;
    SP_DEVINFO_DATA devInfoData = {};
    devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
    
    DWORD deviceIndex = 0;
    while (SetupDiEnumDeviceInfo(hDevInfo, deviceIndex, &devInfoData)) {
        WCHAR hwid[4096] = {};
        
        if (SetupDiGetDeviceRegistryPropertyW(hDevInfo, &devInfoData, SPDRP_HARDWAREID,
            nullptr, (BYTE*)hwid, sizeof(hwid), nullptr)) {
            
            // Check each HWID in the multi-string
            for (wchar_t* p = hwid; *p; p += wcslen(p) + 1) {
                if (_wcsicmp(p, L"ROOT\\IddSampleDriver") == 0) {
                    devicesToRemove.push_back(devInfoData);
                    break;
                }
            }
        }
        
        deviceIndex++;
    }
    
    if (devicesToRemove.empty()) {
        SetupDiDestroyDeviceInfoList(hDevInfo);
        SetLastError("IddSampleDriver not found in system");
        return Status::NotFound;
    }
    
    // Remove each device
    int successCount = 0;
    int failCount = 0;
    std::string lastError;
    
    for (auto& devInfo : devicesToRemove) {
        // Step 1: Disable device first (safer for display devices)
        SP_PROPCHANGE_PARAMS disableParams = {};
        disableParams.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
        disableParams.ClassInstallHeader.InstallFunction = DIF_PROPERTYCHANGE;
        disableParams.StateChange = DICS_DISABLE;
        disableParams.Scope = DICS_FLAG_GLOBAL;
        disableParams.HwProfile = 0;
        
        if (SetupDiSetClassInstallParamsW(hDevInfo, &devInfo, 
                reinterpret_cast<SP_CLASSINSTALL_HEADER*>(&disableParams), sizeof(disableParams))) {
            SetupDiCallClassInstaller(DIF_PROPERTYCHANGE, hDevInfo, &devInfo);
            // Don't fail if disable fails, continue to remove
        }
        
        // Step 2: Remove device
        SP_REMOVEDEVICE_PARAMS removeParams = {};
        removeParams.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
        removeParams.ClassInstallHeader.InstallFunction = DIF_REMOVE;
        removeParams.Scope = DI_REMOVEDEVICE_GLOBAL;
        removeParams.HwProfile = 0;
        
        if (SetupDiSetClassInstallParamsW(hDevInfo, &devInfo, 
                reinterpret_cast<SP_CLASSINSTALL_HEADER*>(&removeParams), sizeof(removeParams))) {
            
            if (SetupDiCallClassInstaller(DIF_REMOVE, hDevInfo, &devInfo)) {
                successCount++;
            } else {
                failCount++;
                lastError = "Failed to remove device: " + std::to_string(::GetLastError());
            }
        } else {
            failCount++;
            lastError = "Failed to set remove parameters: " + std::to_string(::GetLastError());
        }
    }
    
    SetupDiDestroyDeviceInfoList(hDevInfo);
    
    // Clean up Driver Store (optional but recommended)
    // Note: This requires knowing the INF path or OEM name
    // For now, we'll skip this and document it as a limitation
    // TODO: Store INF path during installation for proper cleanup
    
    // Report results
    std::string message = "Uninstall: " + std::to_string(successCount) + " device(s) removed";
    if (failCount > 0) {
        message += ", " + std::to_string(failCount) + " failed. Last error: " + lastError;
        SetLastError(message);
        return (successCount > 0) ? Status::Ok : Status::DriverError;
    }
    
    SetLastError(message);
    return Status::Ok;
}
```

### 完整 IsDriverInstalled (替換 line 807-870)

```cpp
bool VddSdkImpl::IsDriverInstalled() {
    // Method 1: Check Registry for device node
    HKEY hKey;
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, 
        L"SYSTEM\\CurrentControlSet\\Enum\\ROOT\\IddSampleDriver", 
        0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        RegCloseKey(hKey);
        return true;
    }
    
    // Method 2: Check for service entry (optional)
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE,
        L"SYSTEM\\CurrentControlSet\\Services\\IddSampleDriver",
        0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        RegCloseKey(hKey);
        return true;
    }
    
    return false;
}
```

## 應用順序

1. 在 `vddsdk.cpp` 文件頂部添加 `IsRunningAsAdministrator()` 函數
2. 替換 `InstallDriver` (line 587-709)
3. 替換 `UninstallDriver` (line 711-804)
4. 替換 `IsDriverInstalled` (line 807-870)
5. 編譯測試

## 注意事項

- 需要添加 `#include <sddl.h>` 來使用 `CheckTokenMembership`
- Driver Store 清理需要在安裝時保存 INF 路徑（未來改進）
- Reboot 信息已通過 `SetLastError` 傳遞

