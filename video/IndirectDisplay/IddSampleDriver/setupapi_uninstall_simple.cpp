// Simple SetupAPI Driver Uninstallation Tool
#include <windows.h>
#include <setupapi.h>
#include <stdio.h>
#include <vector>
#include <string>

#pragma comment(lib, "setupapi.lib")

int wmain(int argc, wchar_t* argv[]) {
    wprintf(L"========================================\n");
    wprintf(L"SetupAPI Driver Uninstallation Tool\n");
    wprintf(L"========================================\n\n");
    
    // Display adapter class GUID
    GUID displayClassGuid = { 0x4D36E968, 0xE325, 0x11CE, 
        { 0xBF, 0xC1, 0x08, 0x00, 0x2B, 0xE1, 0x03, 0x18 } };
    
    // Get all display devices
    HDEVINFO hDevInfo = SetupDiGetClassDevsW(&displayClassGuid, NULL, NULL, 
        DIGCF_PRESENT | DIGCF_ALLCLASSES);
    
    if (hDevInfo == INVALID_HANDLE_VALUE) {
        wprintf(L"Error: Cannot get device information (Error: %d)\n", GetLastError());
        return 1;
    }
    
    wprintf(L"Scanning for IddSampleDriver devices...\n");
    wprintf(L"----------------------------------------\n");
    
    // Collect all matching devices
    std::vector<SP_DEVINFO_DATA> devicesToRemove;
    SP_DEVINFO_DATA devInfoData = {};
    devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
    
    DWORD deviceIndex = 0;
    while (SetupDiEnumDeviceInfo(hDevInfo, deviceIndex, &devInfoData)) {
        WCHAR deviceId[MAX_PATH] = {};
        WCHAR deviceDesc[MAX_PATH] = {};
        
        // Get Instance ID
        if (SetupDiGetDeviceInstanceIdW(hDevInfo, &devInfoData, deviceId, MAX_PATH, NULL)) {
            // Get Description
            SetupDiGetDeviceRegistryPropertyW(hDevInfo, &devInfoData, SPDRP_DEVICEDESC,
                NULL, (BYTE*)deviceDesc, sizeof(deviceDesc), NULL);
            
            // Check if it's our driver (InstanceId OR Description contains "IddSampleDriver")
            std::wstring deviceIdStr(deviceId);
            std::wstring deviceDescStr(deviceDesc);
            
            if (deviceIdStr.find(L"IddSampleDriver") != std::wstring::npos ||
                deviceIdStr.find(L"IDDSAMPLEDRIVER") != std::wstring::npos ||
                deviceDescStr.find(L"IddSampleDriver") != std::wstring::npos ||
                deviceDescStr.find(L"IDDSAMPLEDRIVER") != std::wstring::npos) {
                
                wprintf(L"Found device:\n");
                wprintf(L"  Instance ID: %s\n", deviceId);
                wprintf(L"  Description: %s\n", deviceDesc);
                wprintf(L"\n");
                
                devicesToRemove.push_back(devInfoData);
            }
        }
        
        deviceIndex++;
    }
    
    if (devicesToRemove.empty()) {
        SetupDiDestroyDeviceInfoList(hDevInfo);
        wprintf(L"No IddSampleDriver devices found\n");
        wprintf(L"\n========================================\n");
        wprintf(L"Uninstall complete (no devices to remove)\n");
        wprintf(L"========================================\n");
        return 0;
    }
    
    wprintf(L"Found %d device(s), starting removal...\n", (int)devicesToRemove.size());
    wprintf(L"----------------------------------------\n\n");
    
    // Remove each device
    int successCount = 0;
    int failCount = 0;
    
    for (size_t i = 0; i < devicesToRemove.size(); i++) {
        wprintf(L"[%d/%d] Removing device...", (int)(i + 1), (int)devicesToRemove.size());
        
        SP_REMOVEDEVICE_PARAMS removeParams = {};
        removeParams.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
        removeParams.ClassInstallHeader.InstallFunction = DIF_REMOVE;
        removeParams.Scope = DI_REMOVEDEVICE_GLOBAL;
        removeParams.HwProfile = 0;
        
        if (SetupDiSetClassInstallParamsW(hDevInfo, &devicesToRemove[i], 
                (PSP_CLASSINSTALL_HEADER)&removeParams, sizeof(removeParams))) {
            
            if (SetupDiCallClassInstaller(DIF_REMOVE, hDevInfo, &devicesToRemove[i])) {
                wprintf(L" Success\n");
                successCount++;
            } else {
                wprintf(L" Failed (Error: %d)\n", GetLastError());
                failCount++;
            }
        } else {
            wprintf(L" Failed - Cannot set removal parameters (Error: %d)\n", GetLastError());
            failCount++;
        }
    }
    
    SetupDiDestroyDeviceInfoList(hDevInfo);
    
    // Summary
    wprintf(L"\n========================================\n");
    wprintf(L"Uninstall Complete\n");
    wprintf(L"========================================\n");
    wprintf(L"Successfully removed: %d device(s)\n", successCount);
    if (failCount > 0) {
        wprintf(L"Failed: %d device(s)\n", failCount);
    }
    wprintf(L"\nPlease verify devices have been removed from Device Manager\n");
    
    return (successCount > 0) ? 0 : 1;
}

