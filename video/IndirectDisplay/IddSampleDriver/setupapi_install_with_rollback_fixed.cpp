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

// Installation state record
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

// Save pre-installation state
bool SavePreInstallState(const wchar_t* filename) {
    wprintf(L"[Backup] Saving pre-installation state...\n");
    
    FILE* fp = NULL;
    if (_wfopen_s(&fp, filename, L"w, ccs=UTF-8") != 0 || fp == NULL) {
        wprintf(L"  Warning: Cannot create backup file\n");
        return false;
    }
    
    // Record timestamp
    SYSTEMTIME st;
    GetLocalTime(&st);
    fwprintf(fp, L"[InstallState]\n");
    fwprintf(fp, L"Timestamp=%04d-%02d-%02d %02d:%02d:%02d\n",
             st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
    
    // List existing display devices
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
    wprintf(L"  OK: State saved to: %s\n", filename);
    return true;
}

// Rollback operation
bool RollbackInstallation(InstallState& state) {
    wprintf(L"\n========================================\n");
    wprintf(L"Performing rollback...\n");
    wprintf(L"========================================\n\n");
    
    bool success = true;
    
    // If driver installed, remove device
    if (state.deviceRegistered && state.deviceInfoSet != INVALID_HANDLE_VALUE) {
        wprintf(L"[Rollback] Removing registered device...\n");
        
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
                wprintf(L"  OK: Device removed\n");
            } else {
                wprintf(L"  FAILED: Remove failed (error: %d)\n", GetLastError());
                success = false;
            }
        }
    }
    
    // Cleanup device info set
    if (state.deviceInfoSet != INVALID_HANDLE_VALUE) {
        SetupDiDestroyDeviceInfoList(state.deviceInfoSet);
        state.deviceInfoSet = INVALID_HANDLE_VALUE;
    }
    
    // Clean registry (if needed)
    if (!state.deviceInstanceId.empty()) {
        wprintf(L"[Rollback] Cleaning registry entries...\n");
        wchar_t regCmd[512];
        swprintf_s(regCmd, L"reg delete \"HKLM\\SYSTEM\\CurrentControlSet\\Enum\\%s\" /f > nul 2>&1",
                   state.deviceInstanceId.c_str());
        _wsystem(regCmd);
        wprintf(L"  Done\n");
    }
    
    // Reset state
    state.deviceCreated = false;
    state.deviceRegistered = false;
    state.driverInstalled = false;
    
    if (success) {
        wprintf(L"\nOK: Rollback successful\n");
    } else {
        wprintf(L"\nFAILED: Errors during rollback, manual cleanup may be required\n");
    }
    
    return success;
}

// Main installation function
bool InstallDriver(const wchar_t* infPath, InstallState& state) {
    wprintf(L"\n========================================\n");
    wprintf(L"Starting driver installation\n");
    wprintf(L"========================================\n\n");
    
    wprintf(L"INF file: %s\n\n", infPath);
    
    // Display adapter class GUID
    GUID displayClassGuid = { 0x4D36E968, 0xE325, 0x11CE, 
        { 0xBF, 0xC1, 0x08, 0x00, 0x2B, 0xE1, 0x03, 0x18 } };
    
    // Step 1: Create device info list
    wprintf(L"[1/5] Creating device info list...\n");
    state.deviceInfoSet = SetupDiCreateDeviceInfoList(&displayClassGuid, NULL);
    
    if (state.deviceInfoSet == INVALID_HANDLE_VALUE) {
        wprintf(L"  FAILED (error: %d)\n", GetLastError());
        return false;
    }
    wprintf(L"  OK\n");
    
    // Step 2: Create device info
    wprintf(L"[2/5] Creating device info...\n");
    
    state.devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
    
    if (!SetupDiCreateDeviceInfoW(
            state.deviceInfoSet,
            L"IddSampleDriver",
            &displayClassGuid,
            L"IddSampleDriver Device",
            NULL,
            DICD_GENERATE_ID,
            &state.devInfoData)) {
        wprintf(L"  FAILED (error: %d)\n", GetLastError());
        return false;
    }
    
    state.deviceCreated = true;
    wprintf(L"  OK\n");
    
    // Step 3: Set hardware ID
    wprintf(L"[3/5] Setting hardware ID...\n");
    
    wchar_t hardwareId[] = L"ROOT\\IddSampleDriver\0\0";
    
    if (!SetupDiSetDeviceRegistryPropertyW(
            state.deviceInfoSet,
            &state.devInfoData,
            SPDRP_HARDWAREID,
            (BYTE*)hardwareId,
            sizeof(hardwareId))) {
        wprintf(L"  FAILED (error: %d)\n", GetLastError());
        return false;
    }
    
    wprintf(L"  OK\n");
    
    // Step 4: Register device
    wprintf(L"[4/5] Registering device...\n");
    
    if (!SetupDiCallClassInstaller(DIF_REGISTERDEVICE, state.deviceInfoSet, &state.devInfoData)) {
        wprintf(L"  FAILED (error: %d)\n", GetLastError());
        return false;
    }
    
    state.deviceRegistered = true;
    
    // Get device instance ID
    wchar_t deviceId[MAX_PATH] = {};
    if (SetupDiGetDeviceInstanceIdW(state.deviceInfoSet, &state.devInfoData, deviceId, MAX_PATH, NULL)) {
        state.deviceInstanceId = deviceId;
        wprintf(L"  OK (ID: %s)\n", deviceId);
    } else {
        wprintf(L"  OK\n");
    }
    
    // Step 5: Install driver
    wprintf(L"[5/5] Installing driver...\n");
    
    if (!SetupDiCallClassInstaller(DIF_INSTALLDEVICE, state.deviceInfoSet, &state.devInfoData)) {
        DWORD error = GetLastError();
        wprintf(L"  FAILED: DIF_INSTALLDEVICE (error: %d)\n", error);
        
        // Try alternative method
        wprintf(L"  Trying UpdateDriverForPlugAndPlayDevices...\n");
        
        BOOL reboot = FALSE;
        if (UpdateDriverForPlugAndPlayDevicesW(
                NULL,
                L"ROOT\\IddSampleDriver",
                infPath,
                INSTALLFLAG_FORCE,
                &reboot)) {
            wprintf(L"  OK (using alternative method)\n");
            state.driverInstalled = true;
            
            if (reboot) {
                wprintf(L"  WARNING: Reboot required\n");
            }
        } else {
            wprintf(L"  FAILED: Alternative method also failed (error: %d)\n", GetLastError());
            return false;
        }
    } else {
        wprintf(L"  OK\n");
        state.driverInstalled = true;
    }
    
    return true;
}

// Verify installation
void VerifyInstallation() {
    wprintf(L"\n========================================\n");
    wprintf(L"Verification\n");
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
                    
                    wprintf(L"OK: Device found: %s\n", deviceDesc);
                    wprintf(L"  Instance ID: %s\n", deviceId);
                    
                    // Check device status
                    ULONG status = 0, problem = 0;
                    if (CM_Get_DevNode_Status(&status, &problem, devInfoData.DevInst, 0) == CR_SUCCESS) {
                        if (problem == 0) {
                            wprintf(L"  Status: Working normally\n");
                        } else {
                            wprintf(L"  Status: Problem (code: %d)\n", problem);
                        }
                    }
                    
                    found = true;
                }
            }
        }
        
        SetupDiDestroyDeviceInfoList(deviceInfoSet);
        
        if (!found) {
            wprintf(L"FAILED: IddSampleDriver device not found\n");
        }
    }
}

// Main function
int wmain(int argc, wchar_t* argv[]) {
    wprintf(L"========================================\n");
    wprintf(L"SetupAPI Driver Install Tool (with Rollback)\n");
    wprintf(L"========================================\n\n");
    
    // Get INF path
    wchar_t infPath[MAX_PATH] = L"IddSampleDriver.inf";
    if (argc > 1) {
        wcscpy_s(infPath, argv[1]);
    }
    
    // Get full path
    wchar_t fullPath[MAX_PATH];
    if (!GetFullPathNameW(infPath, MAX_PATH, fullPath, NULL)) {
        wprintf(L"ERROR: Cannot get full path\n");
        return 1;
    }
    
    // Check file exists
    if (GetFileAttributesW(fullPath) == INVALID_FILE_ATTRIBUTES) {
        wprintf(L"ERROR: INF file not found: %s\n", fullPath);
        return 1;
    }
    
    // Save pre-installation state
    const wchar_t* backupFile = L"driver_install_backup.txt";
    SavePreInstallState(backupFile);
    wprintf(L"\n");
    
    // Installation state
    InstallState state;
    bool installSuccess = false;
    
    // Perform installation
    installSuccess = InstallDriver(fullPath, state);
    
    if (!installSuccess) {
        wprintf(L"\nERROR: Installation failed!\n");
        
        // Ask for rollback
        wprintf(L"\nDo you want to rollback changes? (Y/N): ");
        wchar_t response[10];
        if (fgetws(response, 10, stdin) && (response[0] == L'Y' || response[0] == L'y')) {
            RollbackInstallation(state);
        } else {
            wprintf(L"Skipping rollback. Please clean up manually or run uninstall_driver.exe later\n");
            
            // Cleanup device info set
            if (state.deviceInfoSet != INVALID_HANDLE_VALUE) {
                SetupDiDestroyDeviceInfoList(state.deviceInfoSet);
            }
        }
        
        return 1;
    }
    
    // Verify installation
    VerifyInstallation();
    
    // Cleanup device info set
    if (state.deviceInfoSet != INVALID_HANDLE_VALUE) {
        SetupDiDestroyDeviceInfoList(state.deviceInfoSet);
    }
    
    wprintf(L"\n========================================\n");
    wprintf(L"OK: Installation completed successfully\n");
    wprintf(L"========================================\n");
    wprintf(L"\nBackup file: %s\n", backupFile);
    wprintf(L"To uninstall, run: uninstall_driver.exe\n");
    
    return 0;
}

