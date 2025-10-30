/*++

Copyright (c) 2025

Abstract:

    Test program to install driver using SetupAPI
    
    This demonstrates three methods:
    1. UpdateDriverForPlugAndPlayDevices (newdev.dll)
    2. DiInstallDriver (newdev.dll)
    3. Manual device creation with SetupDiCreateDeviceInfo

Environment:

    User Mode, C++

--*/

#include <windows.h>
#include <setupapi.h>
#include <newdev.h>
#include <cfgmgr32.h>
#include <iostream>
#include <string>

#pragma comment(lib, "setupapi.lib")
#pragma comment(lib, "newdev.lib")

// Check if running as administrator
bool IsRunningAsAdministrator() {
    BOOL isAdmin = FALSE;
    PSID adminGroup = nullptr;
    SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;

    if (AllocateAndInitializeSid(&ntAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &adminGroup)) {
        CheckTokenMembership(nullptr, adminGroup, &isAdmin);
        FreeSid(adminGroup);
    }

    return isAdmin == TRUE;
}

// Print Win32 error message
void PrintLastError(const char* operation, DWORD errorCode) {
    std::cout << "ERROR: " << operation << " failed with error code " << errorCode 
              << " (0x" << std::hex << errorCode << std::dec << ")" << std::endl;
    
    // Print common error meanings
    switch (errorCode) {
        case ERROR_FILE_NOT_FOUND:
            std::cout << "  ERROR_FILE_NOT_FOUND" << std::endl;
            break;
        case ERROR_ACCESS_DENIED:
            std::cout << "  ERROR_ACCESS_DENIED" << std::endl;
            break;
        case ERROR_INVALID_PARAMETER:
            std::cout << "  ERROR_INVALID_PARAMETER" << std::endl;
            break;
        case ERROR_NO_MORE_ITEMS:
            std::cout << "  ERROR_NO_MORE_ITEMS (device not found)" << std::endl;
            break;
        case ERROR_NOT_FOUND:
            std::cout << "  ERROR_NOT_FOUND" << std::endl;
            break;
        default: {
            LPWSTR messageBuffer = nullptr;
            FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                nullptr, errorCode, 0, (LPWSTR)&messageBuffer, 0, nullptr);
            if (messageBuffer) {
                std::wcout << L"  " << messageBuffer << std::endl;
                LocalFree(messageBuffer);
            }
            break;
        }
    }
}

// Method 1: UpdateDriverForPlugAndPlayDevices
bool InstallDriver_Method1(const std::wstring& infPath) {
    std::cout << "\n[Method 1] UpdateDriverForPlugAndPlayDevices" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    
    const wchar_t* hardwareId = L"ROOT\\IddSampleDriver";
    DWORD installFlags = INSTALLFLAG_FORCE | INSTALLFLAG_NONINTERACTIVE;
    BOOL rebootRequired = FALSE;
    
    std::wcout << L"Hardware ID: " << hardwareId << std::endl;
    std::wcout << L"INF Path: " << infPath << std::endl;
    std::cout << "Install Flags: 0x" << std::hex << installFlags << std::dec << std::endl;
    std::cout << std::endl;
    
    BOOL result = UpdateDriverForPlugAndPlayDevicesW(
        nullptr,              // hwndParent
        hardwareId,           // HardwareId
        infPath.c_str(),      // FullInfPath
        installFlags,         // InstallFlags
        &rebootRequired       // bRebootRequired
    );
    
    if (result) {
        std::cout << "SUCCESS: Driver installed via UpdateDriverForPlugAndPlayDevices" << std::endl;
        if (rebootRequired) {
            std::cout << "WARNING: Reboot required" << std::endl;
        }
        return true;
    } else {
        DWORD error = GetLastError();
        PrintLastError("UpdateDriverForPlugAndPlayDevices", error);
        return false;
    }
}

// Method 2: DiInstallDriver
bool InstallDriver_Method2(const std::wstring& infPath) {
    std::cout << "\n[Method 2] DiInstallDriver" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    
    DWORD flags = DIIRFLAG_FORCE_INF;
    BOOL needReboot = FALSE;
    
    std::wcout << L"INF Path: " << infPath << std::endl;
    std::cout << "Flags: 0x" << std::hex << flags << std::dec << std::endl;
    std::cout << std::endl;
    
    BOOL result = DiInstallDriverW(
        nullptr,              // hwndParent
        infPath.c_str(),      // FullInfPath
        flags,                // Flags
        &needReboot           // NeedReboot
    );
    
    if (result) {
        std::cout << "SUCCESS: Driver installed via DiInstallDriver" << std::endl;
        if (needReboot) {
            std::cout << "WARNING: Reboot required" << std::endl;
        }
        return true;
    } else {
        DWORD error = GetLastError();
        PrintLastError("DiInstallDriver", error);
        return false;
    }
}

// Method 3: Manual device creation
bool InstallDriver_Method3(const std::wstring& infPath) {
    std::cout << "\n[Method 3] SetupDiCreateDeviceInfo (Manual)" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    
    // Display adapter class GUID
    GUID displayClassGuid = { 0x4D36E968, 0xE325, 0x11CE, 
        { 0xBF, 0xC1, 0x08, 0x00, 0x2B, 0xE1, 0x03, 0x18 } };
    
    // Step 1: Create device info list
    std::cout << "Step 1: Create device info list..." << std::endl;
    HDEVINFO deviceInfoSet = SetupDiCreateDeviceInfoList(&displayClassGuid, nullptr);
    
    if (deviceInfoSet == INVALID_HANDLE_VALUE) {
        PrintLastError("SetupDiCreateDeviceInfoList", GetLastError());
        return false;
    }
    
    std::cout << "OK: DeviceInfoSet created" << std::endl;
    
    // Step 2: Create device info
    std::cout << "Step 2: Create device info..." << std::endl;
    
    SP_DEVINFO_DATA devInfoData = {};
    devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
    
    BOOL result = SetupDiCreateDeviceInfoW(
        deviceInfoSet,
        L"IddSampleDriver",
        &displayClassGuid,
        L"IddSampleDriver Device",
        nullptr,
        DICD_GENERATE_ID,
        &devInfoData
    );
    
    if (!result) {
        PrintLastError("SetupDiCreateDeviceInfo", GetLastError());
        SetupDiDestroyDeviceInfoList(deviceInfoSet);
        return false;
    }
    
    std::cout << "OK: Device info created" << std::endl;
    
    // Step 3: Set hardware ID
    std::cout << "Step 3: Set hardware ID..." << std::endl;
    
    const wchar_t* hardwareId = L"ROOT\\IddSampleDriver\0\0";
    DWORD hardwareIdSize = (wcslen(hardwareId) + 2) * sizeof(wchar_t);
    
    result = SetupDiSetDeviceRegistryPropertyW(
        deviceInfoSet,
        &devInfoData,
        SPDRP_HARDWAREID,
        (const BYTE*)hardwareId,
        hardwareIdSize
    );
    
    if (!result) {
        PrintLastError("SetupDiSetDeviceRegistryProperty", GetLastError());
        SetupDiDestroyDeviceInfoList(deviceInfoSet);
        return false;
    }
    
    std::cout << "OK: Hardware ID set" << std::endl;
    
    // Step 4: Register device
    std::cout << "Step 4: Register device..." << std::endl;
    
    result = SetupDiCallClassInstaller(DIF_REGISTERDEVICE, deviceInfoSet, &devInfoData);
    
    if (!result) {
        PrintLastError("DIF_REGISTERDEVICE", GetLastError());
        SetupDiDestroyDeviceInfoList(deviceInfoSet);
        return false;
    }
    
    std::cout << "OK: Device registered" << std::endl;
    
    // Step 5: Install device
    std::cout << "Step 5: Install device..." << std::endl;
    
    result = SetupDiCallClassInstaller(DIF_INSTALLDEVICE, deviceInfoSet, &devInfoData);
    
    if (!result) {
        PrintLastError("DIF_INSTALLDEVICE", GetLastError());
        SetupDiDestroyDeviceInfoList(deviceInfoSet);
        return false;
    }
    
    std::cout << "SUCCESS: Device installed!" << std::endl;
    
    // Cleanup
    SetupDiDestroyDeviceInfoList(deviceInfoSet);
    
    return true;
}

// Verify installation
void VerifyInstallation() {
    std::cout << "\n[Verification]" << std::endl;
    std::cout << "========================================" << std::endl;
    
    // Check for device in registry
    std::cout << "Checking registry..." << std::endl;
    
    HKEY hKey;
    const wchar_t* registryPaths[] = {
        L"SYSTEM\\CurrentControlSet\\Services\\WUDFRd",
        L"SYSTEM\\CurrentControlSet\\Services\\IndirectKmd",
        L"SYSTEM\\CurrentControlSet\\Enum\\ROOT\\IddSampleDriver"
    };
    
    for (const auto& path : registryPaths) {
        LONG result = RegOpenKeyExW(HKEY_LOCAL_MACHINE, path, 0, KEY_READ, &hKey);
        if (result == ERROR_SUCCESS) {
            std::wcout << L"  OK: Found " << path << std::endl;
            RegCloseKey(hKey);
        } else {
            std::wcout << L"  NOT FOUND: " << path << std::endl;
        }
    }
    
    // Check for device files
    std::cout << "\nChecking driver files..." << std::endl;
    
    wchar_t systemRoot[MAX_PATH];
    GetEnvironmentVariableW(L"SystemRoot", systemRoot, MAX_PATH);
    
    std::wstring driverPaths[] = {
        std::wstring(systemRoot) + L"\\System32\\IddSampleDriver.dll",
        std::wstring(systemRoot) + L"\\System32\\drivers\\IndirectKmd.sys"
    };
    
    for (const auto& path : driverPaths) {
        DWORD attr = GetFileAttributesW(path.c_str());
        if (attr != INVALID_FILE_ATTRIBUTES) {
            std::wcout << L"  OK: Found " << path << std::endl;
        } else {
            std::wcout << L"  NOT FOUND: " << path << std::endl;
        }
    }
}

int main(int argc, char* argv[]) {
    std::cout << "========================================" << std::endl;
    std::cout << "SetupAPI Driver Installation Test" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;
    
    // Check administrator privileges
    if (!IsRunningAsAdministrator()) {
        std::cout << "ERROR: Administrator privileges required" << std::endl;
        std::cout << "Please run this program as Administrator" << std::endl;
        return 1;
    }
    
    std::cout << "OK: Running as Administrator" << std::endl;
    std::cout << std::endl;
    
    // Get INF path
    std::wstring infPath = L"IddSampleDriver_Fixed.inf";
    
    if (argc > 1) {
        // Convert to wide string
        int len = MultiByteToWideChar(CP_UTF8, 0, argv[1], -1, nullptr, 0);
        if (len > 0) {
            wchar_t* wstr = new wchar_t[len];
            MultiByteToWideChar(CP_UTF8, 0, argv[1], -1, wstr, len);
            infPath = wstr;
            delete[] wstr;
        }
    }
    
    // Check if file exists
    DWORD attr = GetFileAttributesW(infPath.c_str());
    if (attr == INVALID_FILE_ATTRIBUTES) {
        std::wcout << L"ERROR: INF file not found: " << infPath << std::endl;
        return 1;
    }
    
    // Get full path
    wchar_t fullPath[MAX_PATH];
    GetFullPathNameW(infPath.c_str(), MAX_PATH, fullPath, nullptr);
    infPath = fullPath;
    
    std::wcout << L"INF file: " << infPath << std::endl;
    std::cout << std::endl;
    
    // Try each method
    bool success = false;
    
    // Method 1
    if (InstallDriver_Method1(infPath)) {
        success = true;
    }
    
    // Method 2
    if (!success && InstallDriver_Method2(infPath)) {
        success = true;
    }
    
    // Method 3
    if (!success && InstallDriver_Method3(infPath)) {
        success = true;
    }
    
    // Verify
    VerifyInstallation();
    
    std::cout << "\n========================================" << std::endl;
    if (success) {
        std::cout << "Installation Complete - SUCCESS" << std::endl;
    } else {
        std::cout << "Installation Complete - FAILED" << std::endl;
    }
    std::cout << "========================================" << std::endl;
    
    return success ? 0 : 1;
}


