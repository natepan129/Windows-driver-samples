#include <windows.h>
#include <setupapi.h>
#include <newdev.h>
#include <stdio.h>

#pragma comment(lib, "setupapi.lib")
#pragma comment(lib, "newdev.lib")

int wmain(int argc, wchar_t* argv[]) {
    if (argc < 2) {
        wprintf(L"Usage: %s <INF_PATH>\n", argv[0]);
        return 1;
    }

    const wchar_t* infPath = argv[1];
    
    wprintf(L"========================================\n");
    wprintf(L"Modern SetupAPI Driver Installation\n");
    wprintf(L"========================================\n\n");
    wprintf(L"INF File: %s\n\n", infPath);
    
    // Method 1: DiInstallDriver (Windows Vista+)
    wprintf(L"Using DiInstallDriver API...\n");
    
    BOOL needReboot = FALSE;
    if (DiInstallDriverW(NULL, infPath, DIIRFLAG_FORCE_INF, &needReboot)) {
        wprintf(L"SUCCESS! Driver installed via DiInstallDriver\n");
        if (needReboot) {
            wprintf(L"  NOTE: Reboot may be required\n");
        }
        
        // Now create the device
        wprintf(L"\nCreating device instance...\n");
        
        GUID displayClassGuid = { 0x4D36E968, 0xE325, 0x11CE, 
            { 0xBF, 0xC1, 0x08, 0x00, 0x2B, 0xE1, 0x03, 0x18 } };
        
        HDEVINFO deviceInfoSet = SetupDiCreateDeviceInfoList(&displayClassGuid, NULL);
        if (deviceInfoSet != INVALID_HANDLE_VALUE) {
            SP_DEVINFO_DATA devInfoData = {};
            devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
            
            if (SetupDiCreateDeviceInfoW(deviceInfoSet, L"IddSampleDriver", 
                    &displayClassGuid, L"IddSampleDriver Device", NULL, 
                    DICD_GENERATE_ID, &devInfoData)) {
                
                wchar_t hardwareId[] = L"ROOT\\IddSampleDriver\0\0";
                if (SetupDiSetDeviceRegistryPropertyW(deviceInfoSet, &devInfoData,
                        SPDRP_HARDWAREID, (BYTE*)hardwareId, sizeof(hardwareId))) {
                    
                    if (SetupDiCallClassInstaller(DIF_REGISTERDEVICE, deviceInfoSet, &devInfoData)) {
                        wprintf(L"SUCCESS! Device registered\n");
                        
                        wchar_t deviceId[MAX_PATH] = {};
                        if (SetupDiGetDeviceInstanceIdW(deviceInfoSet, &devInfoData, deviceId, MAX_PATH, NULL)) {
                            wprintf(L"  Device ID: %s\n", deviceId);
                        }
                    } else {
                        wprintf(L"FAILED to register device: %d\n", GetLastError());
                    }
                }
            }
            
            SetupDiDestroyDeviceInfoList(deviceInfoSet);
        }
        
        return 0;
    }
    
    DWORD error = GetLastError();
    wprintf(L"FAILED: DiInstallDriver failed with error %d\n", error);
    
    // Method 2: Fallback to UpdateDriverForPlugAndPlayDevices
    wprintf(L"\nTrying UpdateDriverForPlugAndPlayDevices...\n");
    if (UpdateDriverForPlugAndPlayDevicesW(NULL, L"ROOT\\IddSampleDriver",
            infPath, INSTALLFLAG_FORCE, &needReboot)) {
        wprintf(L"SUCCESS! Driver installed via UpdateDriverForPlugAndPlayDevices\n");
        return 0;
    }
    
    wprintf(L"FAILED: All methods failed\n");
    return 1;
}

