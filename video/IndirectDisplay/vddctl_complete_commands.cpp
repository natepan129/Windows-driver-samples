/*++

Copyright (c) 2025 VDD SDK Project

Abstract:

    VDD Command Line Tool (vddctl.exe) - Complete Implementation
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
#include <dxgi.h>
#include <d3d11.h>

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
    
    std::string getOption(const std::string& key, const std::string& defaultValue = "") const {
        auto it = options.find(key);
        return (it != options.end()) ? it->second : defaultValue;
    }
};

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
    std::cout << "  set-mode                Set display mode" << std::endl;
    std::cout << "  set-location            Set display location" << std::endl;
    std::cout << "  set-primary             Set primary display" << std::endl;
    std::cout << "  enumerate               List physical & virtual displays" << std::endl;
    std::cout << "  find-dxgi-output        Find DXGI output by name" << std::endl;
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
    std::cout << "  --inf <path>            Driver INF file path" << std::endl;
    std::cout << "  --hdr                   Enable HDR support" << std::endl;
    std::cout << "  --stereo                Enable stereoscopic 3D" << std::endl;
    std::cout << "  --verbose               Verbose output" << std::endl;
    std::cout << "  --json                  JSON output" << std::endl;
    std::cout << "  --pretty                Pretty-print JSON" << std::endl;
    std::cout << "  --timeout <ms>          Operation timeout" << std::endl;
    std::cout << "  --elevate               Request UAC elevation" << std::endl;
    std::cout << "  --log-level <level>     Log level (trace|debug|info|warn|error)" << std::endl;
    std::cout << std::endl;
    std::cout << "Examples:" << std::endl;
    std::cout << "  vddctl init" << std::endl;
    std::cout << "  vddctl activate --name \"VDD XR\" --width 1920 --height 1080 --refresh 90 --count 1" << std::endl;
    std::cout << "  vddctl set-mode --index 0 --width 2560 --height 1440 --refresh 90" << std::endl;
    std::cout << "  vddctl set-location --index 0 --x 3840 --y 0 --width 1920 --height 1080" << std::endl;
    std::cout << "  vddctl set-primary --index 0" << std::endl;
    std::cout << "  vddctl enumerate" << std::endl;
    std::cout << "  vddctl find-dxgi-output --name \"VDD XR\"" << std::endl;
    std::cout << "  vddctl install --inf \"C:\\Driver\\VDD.inf\"" << std::endl;
    std::cout << "  vddctl uninstall" << std::endl;
    std::cout << "  vddctl deactivate" << std::endl;
    std::cout << "  vddctl shutdown" << std::endl;
}

// Print version information
void printVersion() {
    Version version = GetVersion();
    std::cout << "VDD SDK Version: " << version.major << "." 
              << version.minor << "." << version.patch << std::endl;
    std::cout << "Build Date: " << __DATE__ << " " << __TIME__ << std::endl;
}

// Print status information
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
    if (status == Status::Ok) {
        std::cout << "VDD SDK initialized successfully." << std::endl;
    } else {
        std::cout << "Failed to initialize VDD SDK: " << StatusToString(status) << std::endl;
        if (status != Status::Ok) {
            std::cout << "Error: " << GetLastError() << std::endl;
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
    desc.preferredMode.width = std::stoi(args.getOption("width", "1920"));
    desc.preferredMode.height = std::stoi(args.getOption("height", "1080"));
    desc.preferredMode.refreshNumerator = std::stoi(args.getOption("refresh", "60"));
    desc.preferredMode.refreshDenominator = 1;
    desc.hdr10 = args.hasOption("hdr");
    desc.stereoscopic = args.hasOption("stereo");
    
    uint32_t count = std::stoi(args.getOption("count", "1"));
    
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
            std::cout << "Error: " << GetLastError() << std::endl;
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
            std::cout << "Error: " << GetLastError() << std::endl;
        }
    }
}

// Set display mode
void cmdSetMode(const ArgumentParser& args) {
    std::cout << "Setting display mode..." << std::endl;
    
    uint32_t index = std::stoi(args.getOption("index", "0"));
    DisplayMode mode;
    mode.width = std::stoi(args.getOption("width", "1920"));
    mode.height = std::stoi(args.getOption("height", "1080"));
    mode.refreshNumerator = std::stoi(args.getOption("refresh", "60"));
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
            std::cout << "Error: " << GetLastError() << std::endl;
        }
    }
}

// Set display location
void cmdSetLocation(const ArgumentParser& args) {
    std::cout << "Setting display location..." << std::endl;
    
    uint32_t index = std::stoi(args.getOption("index", "0"));
    DisplayRect rect;
    rect.x = std::stoi(args.getOption("x", "0"));
    rect.y = std::stoi(args.getOption("y", "0"));
    rect.width = std::stoi(args.getOption("width", "1920"));
    rect.height = std::stoi(args.getOption("height", "1080"));
    
    Status status = SetLocation(index, rect);
    if (status == Status::Ok) {
        std::cout << "Display location set successfully." << std::endl;
        std::cout << "Index: " << index << std::endl;
        std::cout << "Position: (" << rect.x << ", " << rect.y << ")" << std::endl;
        std::cout << "Size: " << rect.width << "x" << rect.height << std::endl;
    } else {
        std::cout << "Failed to set display location: " << StatusToString(status) << std::endl;
        if (status != Status::Ok) {
            std::cout << "Error: " << GetLastError() << std::endl;
        }
    }
}

// Set primary display
void cmdSetPrimary(const ArgumentParser& args) {
    std::cout << "Setting primary display..." << std::endl;
    
    uint32_t index = std::stoi(args.getOption("index", "0"));
    
    Status status = SetPrimary(index);
    if (status == Status::Ok) {
        std::cout << "Primary display set successfully." << std::endl;
        std::cout << "Index: " << index << std::endl;
    } else {
        std::cout << "Failed to set primary display: " << StatusToString(status) << std::endl;
        if (status != Status::Ok) {
            std::cout << "Error: " << GetLastError() << std::endl;
        }
    }
}

// Enumerate displays and adapters
void cmdEnumerate(const ArgumentParser& args) {
    std::cout << "Enumerating displays and adapters..." << std::endl;
    
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

// Find DXGI output by name
void cmdFindDxgiOutput(const ArgumentParser& args) {
    std::cout << "Finding DXGI output..." << std::endl;
    
    std::string name = args.getOption("name", "");
    if (name.empty()) {
        std::cout << "Error: Display name is required" << std::endl;
        return;
    }
    
    void* pOutput = nullptr;
    Status status = FindDxgiOutputByName(name, &pOutput);
    if (status == Status::Ok) {
        std::cout << "Found DXGI output: " << name << std::endl;
        std::cout << "Output pointer: " << pOutput << std::endl;
        
        // TODO: Get additional information about the output
        // This would include LUID, rectangle, etc.
    } else {
        std::cout << "Failed to find DXGI output: " << StatusToString(status) << std::endl;
        if (status != Status::Ok) {
            std::cout << "Error: " << GetLastError() << std::endl;
        }
    }
}

// Install driver
void cmdInstall(const ArgumentParser& args) {
    std::cout << "Installing driver..." << std::endl;
    
    std::string infPath = args.getOption("inf", "IddSampleDriver.inf");
    Status status = InstallDriver(std::wstring(infPath.begin(), infPath.end()));
    if (status == Status::Ok) {
        std::cout << "Driver installed successfully." << std::endl;
        std::cout << "INF Path: " << infPath << std::endl;
    } else {
        std::cout << "Failed to install driver: " << StatusToString(status) << std::endl;
        if (status != Status::Ok) {
            std::cout << "Error: " << GetLastError() << std::endl;
        }
    }
}

// Uninstall driver
void cmdUninstall(const ArgumentParser& args) {
    std::cout << "Uninstalling driver..." << std::endl;
    
    Status status = UninstallDriver();
    if (status == Status::Ok) {
        std::cout << "Driver uninstalled successfully." << std::endl;
    } else {
        std::cout << "Failed to uninstall driver: " << StatusToString(status) << std::endl;
        if (status != Status::Ok) {
            std::cout << "Error: " << GetLastError() << std::endl;
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
        } else if (command == "set-mode") {
            cmdSetMode(args);
        } else if (command == "set-location") {
            cmdSetLocation(args);
        } else if (command == "set-primary") {
            cmdSetPrimary(args);
        } else if (command == "enumerate") {
            cmdEnumerate(args);
        } else if (command == "find-dxgi-output") {
            cmdFindDxgiOutput(args);
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
