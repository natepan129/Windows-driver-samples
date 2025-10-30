// Simple SetupAPI Driver Installation Tool
#include <windows.h>
#include <setupapi.h>
#include <newdev.h>
#include <stdio.h>

#pragma comment(lib, "setupapi.lib")
#pragma comment(lib, "newdev.lib")

int wmain(int argc, wchar_t* argv[]) {
    wprintf(L"========================================\n");
    wprintf(L"SetupAPI Driver Installation Tool\n");
    wprintf(L"========================================\n\n");
    
    // Get INF path
    wchar_t infPath[MAX_PATH] = L"IddSampleDriver_Fixed.inf";
    if (argc > 1) {
        wcscpy_s(infPath, argv[1]);
    }
    
    // Get full path
    wchar_t fullPath[MAX_PATH];
    if (!GetFullPathNameW(infPath, MAX_PATH, fullPath, NULL)) {
        wprintf(L"Error: Cannot get full path\n");
        return 1;
    }
    
    // Check if file exists
    if (GetFileAttributesW(fullPath) == INVALID_FILE_ATTRIBUTES) {
        wprintf(L"Error: INF file not found: %s\n", fullPath);
        return 1;
    }
    
    wprintf(L"INF file: %s\n\n", fullPath);
    
    // ========================================
    // Method: Manual device node creation (most reliable)
    // ========================================
    
    wprintf(L"Creating device using SetupDiCreateDeviceInfo...\n");
    wprintf(L"----------------------------------------\n");
    
    // Display adapter class GUID
    GUID displayClassGuid = { 0x4D36E968, 0xE325, 0x11CE, 
        { 0xBF, 0xC1, 0x08, 0x00, 0x2B, 0xE1, 0x03, 0x18 } };
    
    // Step 1: Create device info list
    wprintf(L"[1] Creating device info list...\n");
    HDEVINFO deviceInfoSet = SetupDiCreateDeviceInfoList(&displayClassGuid, NULL);
    
    if (deviceInfoSet == INVALID_HANDLE_VALUE) {
        wprintf(L"Failed: SetupDiCreateDeviceInfoList (Error: %d)\n", GetLastError());
        return 1;
    }
    wprintf(L"Success\n");
    
    // Step 2: Create device info
    wprintf(L"[2] Creating device info...\n");
    
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
        wprintf(L"Failed: SetupDiCreateDeviceInfo (Error: %d)\n", GetLastError());
        SetupDiDestroyDeviceInfoList(deviceInfoSet);
        return 1;
    }
    wprintf(L"Success\n");
    
    // Step 3: Set hardware ID
    wprintf(L"[3] Setting hardware ID...\n");
    
    wchar_t hardwareId[] = L"ROOT\\IddSampleDriver\0\0";
    
    if (!SetupDiSetDeviceRegistryPropertyW(
            deviceInfoSet,
            &devInfoData,
            SPDRP_HARDWAREID,
            (BYTE*)hardwareId,
            sizeof(hardwareId))) {
        wprintf(L"Failed: SetupDiSetDeviceRegistryProperty (Error: %d)\n", GetLastError());
        SetupDiDestroyDeviceInfoList(deviceInfoSet);
        return 1;
    }
    wprintf(L"Success\n");
    
    // Step 4: Register device
    wprintf(L"[4] Registering device...\n");
    
    if (!SetupDiCallClassInstaller(DIF_REGISTERDEVICE, deviceInfoSet, &devInfoData)) {
        wprintf(L"Failed: DIF_REGISTERDEVICE (Error: %d)\n", GetLastError());
        SetupDiDestroyDeviceInfoList(deviceInfoSet);
        return 1;
    }
    wprintf(L"Success\n");
    
    // Step 5: Install driver
    wprintf(L"[5] Installing driver...\n");
    
    if (!SetupDiCallClassInstaller(DIF_INSTALLDEVICE, deviceInfoSet, &devInfoData)) {
        DWORD error = GetLastError();
        wprintf(L"Failed: DIF_INSTALLDEVICE (Error: %d)\n", error);
        
        // Try fallback method even if it fails
        wprintf(L"\nTrying UpdateDriverForPlugAndPlayDevices...\n");
        
        BOOL reboot = FALSE;
        if (UpdateDriverForPlugAndPlayDevicesW(
                NULL,
                L"ROOT\\IddSampleDriver",
                fullPath,
                INSTALLFLAG_FORCE,
                &reboot)) {
            wprintf(L"Success via UpdateDriverForPlugAndPlayDevices\n");
            if (reboot) {
                wprintf(L"Warning: Reboot required\n");
            }
        } else {
            wprintf(L"UpdateDriverForPlugAndPlayDevices also failed (Error: %d)\n", GetLastError());
        }
    } else {
        wprintf(L"Success!\n");
    }
    
    // Cleanup
    SetupDiDestroyDeviceInfoList(deviceInfoSet);
    
    wprintf(L"\n========================================\n");
    wprintf(L"Complete\n");
    wprintf(L"========================================\n");
    wprintf(L"\nPlease check Device Manager to confirm installation\n");
    
    return 0;
}

