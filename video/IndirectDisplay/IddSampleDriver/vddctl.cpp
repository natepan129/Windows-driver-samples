/*++

Copyright (c) 2025 VDD SDK Project

Abstract:

    VDD Command Line Tool (vddctl.exe)
    Command line interface for managing virtual display drivers.

Environment:

    User Mode, C++17

--*/

#include "vddsdk.h"
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <io.h>
#include <Windows.h>

using namespace vdd;

// Command line argument parser
class ArgumentParser {
public:
    std::map<std::string, std::string> options;
    std::vector<std::string> positional;
    
    void parse(int argc, char* argv[]) {
        for (int i = 1; i < argc; i++) {
            std::string arg = argv[i];
            if (arg.substr(0, 2) == "--") {
                // Long option
                std::string key = arg.substr(2);
                if (i + 1 < argc && argv[i + 1][0] != '-') {
                    options[key] = argv[++i];
                } else {
                    options[key] = "true";
                }
            } else if (arg.substr(0, 1) == "-") {
                // Short option
                std::string key = arg.substr(1);
                if (i + 1 < argc && argv[i + 1][0] != '-') {
                    options[key] = argv[++i];
                } else {
                    options[key] = "true";
                }
            } else {
                // Positional argument
                positional.push_back(arg);
            }
        }
    }
    
    bool hasOption(const std::string& key) const {
        return options.find(key) != options.end();
    }
    
    bool hasFlag(const std::string& key) const {
        return hasOption(key);
    }
    
    std::string getOption(const std::string& key, const std::string& defaultValue = "") const {
        auto it = options.find(key);
        return (it != options.end()) ? it->second : defaultValue;
    }
};

// Safe integer parsing with validation
bool safeParseInt(const std::string& str, const std::string& paramName, int32_t& result) {
    if (str.empty()) {
        std::cerr << "Error: Parameter '" << paramName << "' is empty" << std::endl;
        return false;
    }
    
    // Check if string contains only digits (and optional leading minus)
    size_t start = 0;
    if (str[0] == '-') {
        start = 1;
        if (str.length() == 1) {
            std::cerr << "Error: Parameter '" << paramName << "' has invalid value: '" << str << "'" << std::endl;
            return false;
        }
    }
    
    for (size_t i = start; i < str.length(); i++) {
        if (!std::isdigit(static_cast<unsigned char>(str[i]))) {
            std::cerr << "Error: Parameter '" << paramName << "' must be a valid integer, got: '" << str << "'" << std::endl;
            return false;
        }
    }
    
    try {
        result = std::stoi(str);
        return true;
    } catch (const std::out_of_range&) {
        std::cerr << "Error: Parameter '" << paramName << "' value out of range: '" << str << "'" << std::endl;
        return false;
    } catch (const std::exception& e) {
        std::cerr << "Error: Failed to parse parameter '" << paramName << "': " << e.what() << std::endl;
        return false;
    }
}

bool safeParseUInt(const std::string& str, const std::string& paramName, uint32_t& result) {
    int32_t temp;
    if (!safeParseInt(str, paramName, temp)) {
        return false;
    }
    if (temp < 0) {
        std::cerr << "Error: Parameter '" << paramName << "' must be non-negative, got: " << temp << std::endl;
        return false;
    }
    result = static_cast<uint32_t>(temp);
    return true;
}

// Interactive Yes/No prompt (P1 feature)
bool promptYesNo(const std::string& message) {
    // Check if running in interactive terminal
    if (!_isatty(_fileno(stdin))) {
        std::cout << "Non-interactive mode detected - defaulting to NO" << std::endl;
        return false;
    }
    
    std::cout << message << " [y/N]: ";
    std::cout.flush();
    
    std::string response;
    std::getline(std::cin, response);
    
    // Trim whitespace
    response.erase(0, response.find_first_not_of(" \t\n\r"));
    response.erase(response.find_last_not_of(" \t\n\r") + 1);
    
    return (response == "y" || response == "Y" || response == "yes" || response == "YES");
}

// Print help information
void printHelp() {
    std::cout << "VDD Command Line Tool (vddctl.exe)" << std::endl;
    std::cout << "=================================" << std::endl;
    std::cout << std::endl;
    std::cout << "Usage: vddctl <command> [options]" << std::endl;
    std::cout << std::endl;
    std::cout << "Commands:" << std::endl;
    std::cout << "  init                    Initialize VDD SDK" << std::endl;
    std::cout << "  shutdown                Shutdown VDD SDK" << std::endl;
    std::cout << "  status                  Show current status" << std::endl;
    std::cout << "  activate                Activate virtual display" << std::endl;
    std::cout << "  deactivate              Deactivate virtual display" << std::endl;
    std::cout << "  setmode                 Set display mode" << std::endl;
    std::cout << "  setlocation             Set display location" << std::endl;
    std::cout << "  setprimary              Set primary display" << std::endl;
    std::cout << "  list                    List displays and adapters" << std::endl;
    std::cout << "  install                 Install driver" << std::endl;
    std::cout << "  uninstall               Uninstall driver" << std::endl;
    std::cout << "  version                 Show version information" << std::endl;
    std::cout << "  help                    Show this help" << std::endl;
    std::cout << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  --name <name>           Display name" << std::endl;
    std::cout << "  --width <width>         Display width" << std::endl;
    std::cout << "  --height <height>       Display height" << std::endl;
    std::cout << "  --refresh <rate>        Refresh rate" << std::endl;
    std::cout << "  --x <x>                 X position" << std::endl;
    std::cout << "  --y <y>                 Y position" << std::endl;
    std::cout << "  --index <index>         Display index" << std::endl;
    std::cout << "  --count <count>         Number of displays" << std::endl;
    std::cout << "  --verbose               Verbose output" << std::endl;
    std::cout << std::endl;
    std::cout << "Examples:" << std::endl;
    std::cout << "  vddctl init" << std::endl;
    std::cout << "  vddctl activate --name \"Virtual Display\" --width 1920 --height 1080" << std::endl;
    std::cout << "  vddctl setmode --index 0 --width 2560 --height 1440 --refresh 75" << std::endl;
    std::cout << "  vddctl setlocation --index 0 --x 1920 --y 0 --width 1920 --height 1080" << std::endl;
    std::cout << "  vddctl setprimary --index 0 --force-primary" << std::endl;
    std::cout << "  vddctl deactivate" << std::endl;
    std::cout << "  vddctl shutdown" << std::endl;
}

// Print version information
void printVersion() {
    Version version = vdd::GetVersion();
    std::cout << "VDD SDK Version: " << version.major << "." 
              << version.minor << "." << version.patch << std::endl;
    std::cout << "Build Date: " << __DATE__ << " " << __TIME__ << std::endl;
}

// Print status information
// Check if running as administrator
bool IsRunningAsAdmin() {
    BOOL isAdmin = FALSE;
    PSID adminGroup = NULL;
    SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;
    
    if (AllocateAndInitializeSid(&ntAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
                                  DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &adminGroup)) {
        CheckTokenMembership(NULL, adminGroup, &isAdmin);
        FreeSid(adminGroup);
    }
    
    return isAdmin == TRUE;
}

void printStatus() {
    std::cout << "VDD SDK Status:" << std::endl;
    std::cout << "==============" << std::endl;
    
    // Check if driver is installed
    bool driverInstalled = IsDriverInstalled();
    std::cout << "Driver Installed: " << (driverInstalled ? "Yes" : "No") << std::endl;
    
    if (driverInstalled) {
        Version driverVersion = GetDriverVersion();
        std::cout << "Driver Version: " << driverVersion.major << "." 
                  << driverVersion.minor << "." << driverVersion.patch << std::endl;
    }
    
    // Check if display is active
    bool isActive = IsActive();
    std::cout << "Display Active: " << (isActive ? "Yes" : "No") << std::endl;
    
    if (isActive) {
        uint32_t count = GetActiveDisplayCount();
        std::cout << "Active Displays: " << count << std::endl;
    }
    
    // System info
    std::cout << "System Info: " << GetSystemInfo() << std::endl;
}

// Initialize VDD SDK
void cmdInit(const ArgumentParser& args) {
    std::cout << "Initializing VDD SDK..." << std::endl;
    
    SdkConfig config;
    config.enableLogging = args.hasOption("verbose");
    config.maxLogLevel = 2; // Info level
    
    Status status = Initialize(config);
    if (status == Status::Ok || status == Status::AlreadyInstalled) {
        std::cout << "VDD SDK initialized successfully." << std::endl;
    } else {
        std::cout << "Failed to initialize VDD SDK: " << StatusToString(status) << std::endl;
        if (status != Status::Ok) {
            std::cout << "Error: " << vdd::GetLastError() << std::endl;
        }
    }
}

// Shutdown VDD SDK
void cmdShutdown(const ArgumentParser& args) {
    std::cout << "Shutting down VDD SDK..." << std::endl;
    
    Status status = Shutdown();
    if (status == Status::Ok) {
        std::cout << "VDD SDK shutdown successfully." << std::endl;
    } else {
        std::cout << "Failed to shutdown VDD SDK: " << StatusToString(status) << std::endl;
    }
}

// Activate virtual display
void cmdActivate(const ArgumentParser& args) {
    std::cout << "Activating virtual display..." << std::endl;
    
    VirtualDisplayDesc desc;
    desc.name = args.getOption("name", "Virtual Display");
    
    uint32_t width, height, refresh, count;
    if (!safeParseUInt(args.getOption("width", "1920"), "width", width)) return;
    if (!safeParseUInt(args.getOption("height", "1080"), "height", height)) return;
    if (!safeParseUInt(args.getOption("refresh", "60"), "refresh", refresh)) return;
    if (!safeParseUInt(args.getOption("count", "1"), "count", count)) return;
    
    desc.preferredMode.width = width;
    desc.preferredMode.height = height;
    desc.preferredMode.refreshNumerator = refresh;
    desc.preferredMode.refreshDenominator = 1;
    desc.hdr10 = args.hasOption("hdr");
    desc.stereoscopic = args.hasOption("stereo");
    
    Status status = Activate(desc, count);
    if (status == Status::Ok) {
        std::cout << "Virtual display activated successfully." << std::endl;
        std::cout << "Name: " << desc.name << std::endl;
        std::cout << "Resolution: " << desc.preferredMode.width << "x" << desc.preferredMode.height << std::endl;
        std::cout << "Refresh Rate: " << desc.preferredMode.refreshNumerator << "Hz" << std::endl;
        std::cout << "Count: " << count << std::endl;
    } else {
        std::cout << "Failed to activate virtual display: " << StatusToString(status) << std::endl;
        if (status != Status::Ok) {
            std::cout << "Error: " << vdd::GetLastError() << std::endl;
        }
    }
}

// Deactivate virtual display
void cmdDeactivate(const ArgumentParser& args) {
    std::cout << "Deactivating virtual display..." << std::endl;
    
    Status status = Deactivate();
    if (status == Status::Ok) {
        std::cout << "Virtual display deactivated successfully." << std::endl;
    } else {
        std::cout << "Failed to deactivate virtual display: " << StatusToString(status) << std::endl;
        if (status != Status::Ok) {
            std::cout << "Error: " << vdd::GetLastError() << std::endl;
        }
    }
}

// Set display mode
void cmdSetMode(const ArgumentParser& args) {
    std::cout << "Setting display mode..." << std::endl;
    
    uint32_t index, width, height, refresh;
    if (!safeParseUInt(args.getOption("index", "0"), "index", index)) return;
    if (!safeParseUInt(args.getOption("width", "1920"), "width", width)) return;
    if (!safeParseUInt(args.getOption("height", "1080"), "height", height)) return;
    if (!safeParseUInt(args.getOption("refresh", "60"), "refresh", refresh)) return;
    
    DisplayMode mode;
    mode.width = width;
    mode.height = height;
    mode.refreshNumerator = refresh;
    mode.refreshDenominator = 1;
    
    Status status = SetMode(index, mode);
    if (status == Status::Ok) {
        std::cout << "Display mode set successfully." << std::endl;
        std::cout << "Index: " << index << std::endl;
        std::cout << "Resolution: " << mode.width << "x" << mode.height << std::endl;
        std::cout << "Refresh Rate: " << mode.refreshNumerator << "Hz" << std::endl;
    } else {
        std::cout << "Failed to set display mode: " << StatusToString(status) << std::endl;
        if (status != Status::Ok) {
            std::cout << "Error: " << vdd::GetLastError() << std::endl;
        }
    }
}

// Set display location
void cmdSetLocation(const ArgumentParser& args) {
    std::cout << "Setting display location..." << std::endl;
    
    uint32_t index, width, height;
    int32_t x, y;
    if (!safeParseUInt(args.getOption("index", "0"), "index", index)) return;
    if (!safeParseInt(args.getOption("x", "0"), "x", x)) return;
    if (!safeParseInt(args.getOption("y", "0"), "y", y)) return;
    if (!safeParseUInt(args.getOption("width", "1920"), "width", width)) return;
    if (!safeParseUInt(args.getOption("height", "1080"), "height", height)) return;
    
    DisplayRect rect;
    rect.x = x;
    rect.y = y;
    rect.width = width;
    rect.height = height;
    
    Status status = SetLocation(index, rect);
    if (status == Status::Ok) {
        std::cout << "Display location set successfully." << std::endl;
        std::cout << "Index: " << index << std::endl;
        std::cout << "Position: (" << rect.x << ", " << rect.y << ")" << std::endl;
        std::cout << "Size: " << rect.width << "x" << rect.height << std::endl;
    } else {
        std::cout << "Failed to set display location: " << StatusToString(status) << std::endl;
        if (status != Status::Ok) {
            std::cout << "Error: " << vdd::GetLastError() << std::endl;
        }
    }
}

// Set primary display (with P1+P2 features)
void cmdSetPrimary(const ArgumentParser& args) {
    std::cout << "Setting primary display..." << std::endl;
    std::cout << "========================================" << std::endl;
    
    // Check for flags (ArgumentParser stores keys without -- prefix)
    bool force = args.hasFlag("force-primary");
    bool yes = args.hasFlag("yes") || args.hasFlag("y");
    bool dryRun = args.hasFlag("dry-run");
    
    uint32_t index;
    if (!safeParseUInt(args.getOption("index", "0"), "index", index)) return;
    
    // ============================================================================
    // P0: Force flag required
    // ============================================================================
    if (!force) {
        std::cout << "⚠️  WARNING: SetPrimary is DANGEROUS!" << std::endl;
        std::cout << "This operation requires the --force-primary flag." << std::endl;
        std::cout << std::endl;
        std::cout << "Risks:" << std::endl;
        std::cout << "  • May cause login screen issues" << std::endl;
        std::cout << "  • May cause black screen on reboot" << std::endl;
        std::cout << "  • May lock you out of the system" << std::endl;
        std::cout << std::endl;
        std::cout << "Usage: vddctl setprimary <index> --force-primary [--yes] [--dry-run]" << std::endl;
        std::cout << std::endl;
        std::cout << "Options:" << std::endl;
        std::cout << "  --force-primary  Override safety check (required)" << std::endl;
        std::cout << "  --yes, -y        Skip interactive confirmation" << std::endl;
        std::cout << "  --dry-run        Check only, don't execute" << std::endl;
        std::cout << "========================================" << std::endl;
        return;
    }
    
    // ============================================================================
    // P1: Display information and target details
    // ============================================================================
    std::cout << "Target:" << std::endl;
    std::cout << "  Output Index: " << index << std::endl;
    std::cout << std::endl;
    
    std::cout << "Safety Checks:" << std::endl;
    std::cout << "  ✓ Force flag provided" << std::endl;
    std::cout << "  ⏳ Remote/VM check (will be performed by SDK)" << std::endl;
    std::cout << "  ⏳ Physical display check (will be performed by SDK)" << std::endl;
    std::cout << "  ⏳ Target visibility check (will be performed by SDK)" << std::endl;
    std::cout << "  ⏳ Topology backup (will be performed by SDK)" << std::endl;
    std::cout << std::endl;
    
    // ============================================================================
    // P1: Dry-run mode (check only, don't execute)
    // ============================================================================
    if (dryRun) {
        std::cout << "🔍 DRY-RUN MODE: Checking feasibility without executing..." << std::endl;
        std::cout << "========================================" << std::endl;
        
        std::cout << "Checks that would be performed:" << std::endl;
        std::cout << "  1. ✓ Force flag check - PASSED" << std::endl;
        std::cout << "  2. Remote/VM session check" << std::endl;
        std::cout << "  3. Physical display availability check" << std::endl;
        std::cout << "  4. Administrator privileges check" << std::endl;
        std::cout << "  5. Target display visibility check" << std::endl;
        std::cout << "  6. Topology backup" << std::endl;
        std::cout << "  7. Execute SetPrimary operation" << std::endl;
        std::cout << "  8. Automatic rollback on failure" << std::endl;
        std::cout << std::endl;
        std::cout << "✓ Dry-run complete. Use without --dry-run to execute." << std::endl;
        std::cout << "========================================" << std::endl;
        return;
    }
    
    // ============================================================================
    // P1: Interactive confirmation (unless --yes provided)
    // ============================================================================
    if (!yes) {
        std::cout << "⚠️  FINAL CONFIRMATION REQUIRED ⚠️" << std::endl;
        std::cout << std::endl;
        std::cout << "You are about to set a VIRTUAL display as PRIMARY!" << std::endl;
        std::cout << "This is a HIGH-RISK operation that may:" << std::endl;
        std::cout << "  • Cause black screen on reboot" << std::endl;
        std::cout << "  • Make login screen inaccessible" << std::endl;
        std::cout << "  • Require safe mode to recover" << std::endl;
        std::cout << std::endl;
        std::cout << "Recovery methods if something goes wrong:" << std::endl;
        std::cout << "  1. Press Ctrl+Alt+Del → Task Manager → vddctl deactivate" << std::endl;
        std::cout << "  2. Run: displayswitch.exe /internal" << std::endl;
        std::cout << "  3. Reboot to Safe Mode" << std::endl;
        std::cout << std::endl;
        
        if (!promptYesNo("Do you really want to proceed?")) {
            std::cout << std::endl;
            std::cout << "Operation cancelled by user." << std::endl;
            std::cout << "========================================" << std::endl;
            return;
        }
        
        std::cout << std::endl;
        std::cout << "User confirmed. Proceeding..." << std::endl;
    } else {
        std::cout << "⚠️  --yes flag detected - skipping interactive confirmation" << std::endl;
    }
    
    std::cout << "========================================" << std::endl;
    std::cout << "Executing SetPrimary with full safety checks..." << std::endl;
    std::cout << "========================================" << std::endl;
    
    // ============================================================================
    // Execute the operation
    // ============================================================================
    Status status = SetPrimary(index, force);
    
    // ============================================================================
    // Handle results
    // ============================================================================
    if (status == Status::Ok) {
        std::cout << std::endl;
        std::cout << "========================================" << std::endl;
        std::cout << "✓ SUCCESS: Primary display set!" << std::endl;
        std::cout << "========================================" << std::endl;
        std::cout << "Output Index: " << index << std::endl;
        std::cout << "Status: Active" << std::endl;
        std::cout << std::endl;
        std::cout << "Please verify:" << std::endl;
        std::cout << "  • Taskbar moved to virtual display?" << std::endl;
        std::cout << "  • New windows open on virtual display?" << std::endl;
        std::cout << "  • Login screen still accessible?" << std::endl;
        std::cout << "========================================" << std::endl;
    } else if (status == Status::OperationNotPermitted) {
        std::cout << "✗ Operation not permitted: " << vdd::GetLastError() << std::endl;
        std::cout << "Hint: This shouldn't happen with --force-primary flag." << std::endl;
        std::cout << "========================================" << std::endl;
    } else if (status == Status::AccessDenied) {
        std::cout << "✗ Access denied: " << vdd::GetLastError() << std::endl;
        std::cout << "This operation is blocked in remote/VM sessions for safety." << std::endl;
        std::cout << "Cannot be overridden - please use a physical machine console." << std::endl;
        std::cout << "========================================" << std::endl;
    } else {
        std::cout << "✗ Failed to set primary display: " << StatusToString(status) << std::endl;
        std::cout << "Error: " << vdd::GetLastError() << std::endl;
        std::cout << std::endl;
        std::cout << "Note: Topology should have been automatically restored." << std::endl;
        std::cout << std::endl;
        std::cout << "If you still have issues, try:" << std::endl;
        std::cout << "  1. Press Ctrl+Alt+Del" << std::endl;
        std::cout << "  2. Open Task Manager" << std::endl;
        std::cout << "  3. Run: vddctl deactivate" << std::endl;
        std::cout << "  4. Or run: displayswitch.exe /internal" << std::endl;
        std::cout << "========================================" << std::endl;
    }
}

// List displays and adapters
void cmdList(const ArgumentParser& args) {
    std::cout << "Listing displays and adapters..." << std::endl;
    
    // List adapters
    std::vector<AdapterInfo> adapters;
    Status status = EnumerateAdapters(adapters);
    if (status == Status::Ok) {
        std::cout << "Found " << adapters.size() << " display adapters:" << std::endl;
        for (size_t i = 0; i < adapters.size(); ++i) {
            const auto& adapter = adapters[i];
            std::cout << "  [" << i << "] " << adapter.name;
            if (adapter.isVirtual) {
                std::cout << " (Virtual)";
            }
            if (adapter.isActive) {
                std::cout << " (Active)";
            }
            if (adapter.isPrimary) {
                std::cout << " (Primary)";
            }
            std::cout << std::endl;
        }
    } else {
        std::cout << "Failed to enumerate adapters: " << StatusToString(status) << std::endl;
    }
    
    // List active displays
    if (IsActive()) {
        uint32_t count = GetActiveDisplayCount();
        std::cout << "Active displays: " << count << std::endl;
        
        for (uint32_t i = 0; i < count; ++i) {
            DisplayMode mode;
            DisplayRect rect;
            if (GetMode(i, mode) == Status::Ok) {
                std::cout << "  Display " << i << ": " << mode.width << "x" << mode.height 
                          << "@" << (mode.refreshNumerator / mode.refreshDenominator) << "Hz" << std::endl;
            }
            if (GetLocation(i, rect) == Status::Ok) {
                std::cout << "    Position: (" << rect.x << ", " << rect.y << ")" << std::endl;
                std::cout << "    Size: " << rect.width << "x" << rect.height << std::endl;
            }
        }
    }
}

// Install driver
void cmdInstall(const ArgumentParser& args) {
    std::cout << "Installing driver..." << std::endl;
    std::cout << "========================================" << std::endl;
    
    // Check for administrator privileges
    if (!IsRunningAsAdmin()) {
        std::cout << "ERROR: Administrator privileges required!" << std::endl;
        std::cout << std::endl;
        std::cout << "Please run this command as administrator:" << std::endl;
        std::cout << "  Right-click -> Run as administrator" << std::endl;
        std::cout << "  Or use: Start-Process -Verb RunAs" << std::endl;
        std::cout << "========================================" << std::endl;
        return;
    }
    std::cout << "Administrator check: OK" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    
    // Note: User must call 'vddctl init' before install (design doc requirement)
    std::cout << "Note: Please ensure 'vddctl init' was called first" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    
    // Get INF path from positional argument (vddctl install <inf_path>)
    std::string infPath = args.positional.size() > 1 ? args.positional[1] : "IddSampleDriver.inf";
    std::cout << "INF Path (raw): " << infPath << std::endl;
    
    // Convert to wstring (UTF-8 to UTF-16 conversion for non-ASCII paths)
    int size = MultiByteToWideChar(CP_UTF8, 0, infPath.c_str(), -1, nullptr, 0);
    if (size <= 0) {
        std::cout << "ERROR: Failed to convert INF path encoding" << std::endl;
        return;
    }
    std::wstring winfPath(size, 0);
    MultiByteToWideChar(CP_UTF8, 0, infPath.c_str(), -1, &winfPath[0], size);
    winfPath.resize(size - 1);  // Remove null terminator
    std::wcout << L"INF Path (wide): " << winfPath << std::endl;
    
    // Get absolute path
    wchar_t absPath[MAX_PATH];
    DWORD result = GetFullPathNameW(winfPath.c_str(), MAX_PATH, absPath, nullptr);
    if (result == 0) {
        std::cout << "ERROR: Failed to get absolute path" << std::endl;
        return;
    }
    std::wcout << L"Absolute Path: " << absPath << std::endl;
    
    // Check if file exists
    DWORD fileAttr = GetFileAttributesW(absPath);
    if (fileAttr == INVALID_FILE_ATTRIBUTES) {
        std::cout << "ERROR: INF file not found!" << std::endl;
        std::cout << "Please check the file path." << std::endl;
        return;
    } else {
        std::cout << "File exists: OK" << std::endl;
    }
    
    std::cout << "----------------------------------------" << std::endl;
    std::cout << "Calling InstallDriver() with absolute path..." << std::endl;
    
    // Use absolute path
    Status status = InstallDriver(absPath);
    
    std::cout << "----------------------------------------" << std::endl;
    if (status == Status::Ok) {
        std::cout << "Driver installed successfully." << std::endl;
    } else if (status == Status::RebootRequired) {
        std::cout << "Driver installed successfully." << std::endl;
        std::cout << "========================================" << std::endl;
        std::cout << "*** REBOOT REQUIRED ***" << std::endl;
        std::cout << "========================================" << std::endl;
        std::cout << "Please restart your computer to complete the installation." << std::endl;
    } else {
        std::cout << "Failed to install driver: " << StatusToString(status) << std::endl;
        std::cout << "Error details: " << vdd::GetLastError() << std::endl;
    }
    std::cout << "========================================" << std::endl;
}

// Uninstall driver
void cmdUninstall(const ArgumentParser& args) {
    std::cout << "Uninstalling driver..." << std::endl;
    std::cout << "========================================" << std::endl;
    
    // Check for administrator privileges
    bool hasAdmin = IsRunningAsAdmin();
    std::cout << "Administrator check: " << (hasAdmin ? "OK" : "WARNING - No admin") << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    if (!hasAdmin) {
        std::cout << "Note: Some operations may fail without admin privileges" << std::endl;
    }
    std::cout << "----------------------------------------" << std::endl;
    
    // Safety check: Detect physical displays
    std::cout << "Checking for physical displays..." << std::endl;
    
    // Enumerate adapters to check for physical displays
    std::vector<AdapterInfo> adapters;
    Status enumStatus = EnumerateAdapters(adapters);
    
    bool hasPhysicalDisplay = false;
    int physicalCount = 0;
    int virtualCount = 0;
    
    if (enumStatus == Status::Ok) {
        for (const auto& adapter : adapters) {
            if (adapter.isActive) {
                if (adapter.isVirtual) {
                    virtualCount++;
                } else {
                    hasPhysicalDisplay = true;
                    physicalCount++;
                }
            }
        }
        
        std::cout << "Physical displays: " << physicalCount << std::endl;
        std::cout << "Virtual displays: " << virtualCount << std::endl;
    } else {
        std::cout << "WARNING: Could not enumerate displays" << std::endl;
    }
    
    // If no physical display, require confirmation
    if (!hasPhysicalDisplay) {
        std::cout << "========================================" << std::endl;
        std::cout << "⚠️  WARNING: No physical display detected!" << std::endl;
        std::cout << "========================================" << std::endl;
        std::cout << "Uninstalling the virtual display driver without a physical display" << std::endl;
        std::cout << "may leave your system without any active displays!" << std::endl;
        std::cout << std::endl;
        
        bool yes = args.hasFlag("yes") || args.hasFlag("y");
        if (!yes) {
            std::cout << "Use --yes flag to confirm this risky operation." << std::endl;
            std::cout << "Or connect a physical display before uninstalling." << std::endl;
            std::cout << "========================================" << std::endl;
            std::cout << "Uninstall cancelled for safety." << std::endl;
            return;
        } else {
            std::cout << "WARNING: Proceeding anyway due to --yes flag" << std::endl;
        }
    }
    
    std::cout << "----------------------------------------" << std::endl;
    
    // Note: User must call 'vddctl init' before uninstall (design doc requirement)
    std::cout << "Note: Please ensure 'vddctl init' was called first" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    
    Status status = vdd::UninstallDriver();
    if (status == Status::Ok) {
        std::cout << "Driver uninstalled successfully." << std::endl;
    } else {
        std::cout << "Failed to uninstall driver: " << StatusToString(status) << std::endl;
        if (status != Status::Ok) {
            std::cout << "Error: " << vdd::GetLastError() << std::endl;
        }
    }
}

// Main function
int main(int argc, char* argv[]) {
    if (argc < 2) {
        printHelp();
        return 1;
    }
    
    ArgumentParser args;
    args.parse(argc, argv);
    
    std::string command = args.positional.empty() ? "" : args.positional[0];
    
    try {
        if (command == "init") {
            cmdInit(args);
        } else if (command == "shutdown") {
            cmdShutdown(args);
        } else if (command == "status") {
            printStatus();
        } else if (command == "activate") {
            cmdActivate(args);
        } else if (command == "deactivate") {
            cmdDeactivate(args);
        } else if (command == "setmode") {
            cmdSetMode(args);
        } else if (command == "setlocation") {
            cmdSetLocation(args);
        } else if (command == "setprimary") {
            cmdSetPrimary(args);
        } else if (command == "list") {
            cmdList(args);
        } else if (command == "install") {
            cmdInstall(args);
        } else if (command == "uninstall") {
            cmdUninstall(args);
        } else if (command == "version") {
            printVersion();
        } else if (command == "help") {
            printHelp();
        } else {
            std::cout << "Unknown command: " << command << std::endl;
            std::cout << "Use 'vddctl help' for usage information." << std::endl;
            return 1;
        }
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}

