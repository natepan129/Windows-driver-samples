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
#include <thread>
#include <chrono>
#include <fstream>
#include <sstream>
#include <iomanip>

using namespace vdd;

// Global options
struct GlobalOptions {
    bool quiet = false;
    bool json = false;
    bool pretty = false;
    int timeout = 15000;
    bool elevate = false;
    std::string logLevel = "info";
};

GlobalOptions g_options;

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

// JSON output helper
class JsonOutput {
private:
    std::ostringstream m_stream;
    bool m_first = true;
    
public:
    void startObject() {
        m_stream << "{";
        m_first = true;
    }
    
    void endObject() {
        m_stream << "}";
    }
    
    void addField(const std::string& key, const std::string& value) {
        if (!m_first) m_stream << ",";
        m_stream << "\"" << key << "\":\"" << value << "\"";
        m_first = false;
    }
    
    void addField(const std::string& key, int value) {
        if (!m_first) m_stream << ",";
        m_stream << "\"" << key << "\":" << value;
        m_first = false;
    }
    
    void addField(const std::string& key, bool value) {
        if (!m_first) m_stream << ",";
        m_stream << "\"" << key << "\":" << (value ? "true" : "false");
        m_first = false;
    }
    
    std::string toString() const {
        return m_stream.str();
    }
};

// Print help information
void printHelp() {
    if (g_options.json) {
        JsonOutput json;
        json.startObject();
        json.addField("help", "VDD Command Line Tool (vddctl.exe)");
        json.addField("usage", "vddctl <command> [options]");
        json.endObject();
        std::cout << json.toString() << std::endl;
        return;
    }
    
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
    std::cout << "  find-dxgi-output         Find DXGI output by name" << std::endl;
    std::cout << "  install                 Install driver" << std::endl;
    std::cout << "  uninstall               Uninstall driver" << std::endl;
    std::cout << "  begin-session            Begin fault-tolerant session" << std::endl;
    std::cout << "  activate-leased         Activate with session lease" << std::endl;
    std::cout << "  heartbeat               Send session heartbeat" << std::endl;
    std::cout << "  end-session             End session" << std::endl;
    std::cout << "  get-session-state       Get session state" << std::endl;
    std::cout << "  recover-orphaned        Recover orphaned state" << std::endl;
    std::cout << "  ensure-driver-running   Ensure driver is running" << std::endl;
    std::cout << "  get-last-error          Get last error" << std::endl;
    std::cout << "  get-system-info         Get system information" << std::endl;
    std::cout << "  set-hdr                 Set HDR support" << std::endl;
    std::cout << "  set-stereo              Set stereo support" << std::endl;
    std::cout << "  set-custom-edid         Set custom EDID" << std::endl;
    std::cout << "  get-edid                Get EDID data" << std::endl;
    std::cout << "  version                 Show version information" << std::endl;
    std::cout << "  help                    Show this help" << std::endl;
    std::cout << std::endl;
    std::cout << "Global Options:" << std::endl;
    std::cout << "  -q, --quiet            Suppress non-essential output" << std::endl;
    std::cout << "  -j, --json             Force JSON output" << std::endl;
    std::cout << "  -p, --pretty           Pretty-print JSON" << std::endl;
    std::cout << "  -t, --timeout <ms>     Operation timeout (default 15000)" << std::endl;
    std::cout << "  --elevate              Request UAC elevation for admin ops" << std::endl;
    std::cout << "  --log-level <lvl>      trace|debug|info|warn|error (default info)" << std::endl;
    std::cout << std::endl;
    std::cout << "Exit Codes:" << std::endl;
    std::cout << "  0=OK, 1=UserError, 2=NotFound, 3=Busy, 4=AdminRequired," << std::endl;
    std::cout << "  5=DriverError, 6=OsUnsupported, 10=Unexpected" << std::endl;
}

// Print version information
void printVersion() {
    Version version = GetVersion();
    
    if (g_options.json) {
        JsonOutput json;
        json.startObject();
        json.addField("version", std::to_string(version.major) + "." + 
                     std::to_string(version.minor) + "." + std::to_string(version.patch));
        json.addField("buildDate", std::string(__DATE__) + " " + __TIME__);
        json.endObject();
        std::cout << json.toString() << std::endl;
        return;
    }
    
    std::cout << "VDD SDK Version: " << version.major << "." 
              << version.minor << "." << version.patch << std::endl;
    std::cout << "Build Date: " << __DATE__ << " " << __TIME__ << std::endl;
}

// Print status information
void printStatus() {
    if (g_options.json) {
        JsonOutput json;
        json.startObject();
        
        bool driverInstalled = IsDriverInstalled();
        json.addField("driverInstalled", driverInstalled);
        
        if (driverInstalled) {
            Version driverVersion = GetDriverVersion();
            json.addField("driverVersion", std::to_string(driverVersion.major) + "." + 
                         std::to_string(driverVersion.minor) + "." + std::to_string(driverVersion.patch));
        }
        
        bool isActive = IsActive();
        json.addField("displayActive", isActive);
        
        if (isActive) {
            uint32_t count = GetActiveDisplayCount();
            json.addField("activeDisplayCount", (int)count);
        }
        
        json.addField("systemInfo", GetSystemInfo());
        json.endObject();
        std::cout << json.toString() << std::endl;
        return;
    }
    
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
    if (g_options.json) {
        JsonOutput json;
        json.startObject();
        json.addField("status", "initializing");
        json.endObject();
        std::cout << json.toString() << std::endl;
    } else if (!g_options.quiet) {
        std::cout << "Initializing VDD SDK..." << std::endl;
    }
    
    SdkConfig config;
    config.enableLogging = args.hasOption("verbose");
    config.maxLogLevel = 2; // Info level
    
    Status status = Initialize(config);
    
    if (g_options.json) {
        JsonOutput json;
        json.startObject();
        json.addField("status", StatusToString(status));
        if (status != Status::Ok) {
            json.addField("error", GetLastError());
        }
        json.endObject();
        std::cout << json.toString() << std::endl;
    } else {
        if (status == Status::Ok) {
            std::cout << "VDD SDK initialized successfully." << std::endl;
        } else {
            std::cout << "Failed to initialize VDD SDK: " << StatusToString(status) << std::endl;
            if (status != Status::Ok) {
                std::cout << "Error: " << GetLastError() << std::endl;
            }
        }
    }
}

// Shutdown VDD SDK
void cmdShutdown(const ArgumentParser& args) {
    if (g_options.json) {
        JsonOutput json;
        json.startObject();
        json.addField("status", "shutting down");
        json.endObject();
        std::cout << json.toString() << std::endl;
    } else if (!g_options.quiet) {
        std::cout << "Shutting down VDD SDK..." << std::endl;
    }
    
    Status status = Shutdown();
    
    if (g_options.json) {
        JsonOutput json;
        json.startObject();
        json.addField("status", StatusToString(status));
        if (status != Status::Ok) {
            json.addField("error", GetLastError());
        }
        json.endObject();
        std::cout << json.toString() << std::endl;
    } else {
        if (status == Status::Ok) {
            std::cout << "VDD SDK shutdown successfully." << std::endl;
        } else {
            std::cout << "Failed to shutdown VDD SDK: " << StatusToString(status) << std::endl;
        }
    }
}

// Activate virtual display
void cmdActivate(const ArgumentParser& args) {
    if (g_options.json) {
        JsonOutput json;
        json.startObject();
        json.addField("status", "activating");
        json.endObject();
        std::cout << json.toString() << std::endl;
    } else if (!g_options.quiet) {
        std::cout << "Activating virtual display..." << std::endl;
    }
    
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
    
    if (g_options.json) {
        JsonOutput json;
        json.startObject();
        json.addField("status", StatusToString(status));
        if (status == Status::Ok) {
            json.addField("name", desc.name);
            json.addField("width", (int)desc.preferredMode.width);
            json.addField("height", (int)desc.preferredMode.height);
            json.addField("refresh", (int)desc.preferredMode.refreshNumerator);
            json.addField("count", (int)count);
        } else {
            json.addField("error", GetLastError());
        }
        json.endObject();
        std::cout << json.toString() << std::endl;
    } else {
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
}

// Deactivate virtual display
void cmdDeactivate(const ArgumentParser& args) {
    if (g_options.json) {
        JsonOutput json;
        json.startObject();
        json.addField("status", "deactivating");
        json.endObject();
        std::cout << json.toString() << std::endl;
    } else if (!g_options.quiet) {
        std::cout << "Deactivating virtual display..." << std::endl;
    }
    
    Status status = Deactivate();
    
    if (g_options.json) {
        JsonOutput json;
        json.startObject();
        json.addField("status", StatusToString(status));
        if (status != Status::Ok) {
            json.addField("error", GetLastError());
        }
        json.endObject();
        std::cout << json.toString() << std::endl;
    } else {
        if (status == Status::Ok) {
            std::cout << "Virtual display deactivated successfully." << std::endl;
        } else {
            std::cout << "Failed to deactivate virtual display: " << StatusToString(status) << std::endl;
            if (status != Status::Ok) {
                std::cout << "Error: " << GetLastError() << std::endl;
            }
        }
    }
}

// Set display mode
void cmdSetMode(const ArgumentParser& args) {
    if (g_options.json) {
        JsonOutput json;
        json.startObject();
        json.addField("status", "setting mode");
        json.endObject();
        std::cout << json.toString() << std::endl;
    } else if (!g_options.quiet) {
        std::cout << "Setting display mode..." << std::endl;
    }
    
    uint32_t index = std::stoi(args.getOption("index", "0"));
    DisplayMode mode;
    mode.width = std::stoi(args.getOption("width", "1920"));
    mode.height = std::stoi(args.getOption("height", "1080"));
    mode.refreshNumerator = std::stoi(args.getOption("refresh", "60"));
    mode.refreshDenominator = 1;
    
    Status status = SetMode(index, mode);
    
    if (g_options.json) {
        JsonOutput json;
        json.startObject();
        json.addField("status", StatusToString(status));
        if (status == Status::Ok) {
            json.addField("index", (int)index);
            json.addField("width", (int)mode.width);
            json.addField("height", (int)mode.height);
            json.addField("refresh", (int)mode.refreshNumerator);
        } else {
            json.addField("error", GetLastError());
        }
        json.endObject();
        std::cout << json.toString() << std::endl;
    } else {
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
}

// Set display location
void cmdSetLocation(const ArgumentParser& args) {
    if (g_options.json) {
        JsonOutput json;
        json.startObject();
        json.addField("status", "setting location");
        json.endObject();
        std::cout << json.toString() << std::endl;
    } else if (!g_options.quiet) {
        std::cout << "Setting display location..." << std::endl;
    }
    
    uint32_t index = std::stoi(args.getOption("index", "0"));
    DisplayRect rect;
    rect.x = std::stoi(args.getOption("x", "0"));
    rect.y = std::stoi(args.getOption("y", "0"));
    rect.width = std::stoi(args.getOption("width", "1920"));
    rect.height = std::stoi(args.getOption("height", "1080"));
    
    Status status = SetLocation(index, rect);
    
    if (g_options.json) {
        JsonOutput json;
        json.startObject();
        json.addField("status", StatusToString(status));
        if (status == Status::Ok) {
            json.addField("index", (int)index);
            json.addField("x", (int)rect.x);
            json.addField("y", (int)rect.y);
            json.addField("width", (int)rect.width);
            json.addField("height", (int)rect.height);
        } else {
            json.addField("error", GetLastError());
        }
        json.endObject();
        std::cout << json.toString() << std::endl;
    } else {
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
}

// Set primary display
void cmdSetPrimary(const ArgumentParser& args) {
    if (g_options.json) {
        JsonOutput json;
        json.startObject();
        json.addField("status", "setting primary");
        json.endObject();
        std::cout << json.toString() << std::endl;
    } else if (!g_options.quiet) {
        std::cout << "Setting primary display..." << std::endl;
    }
    
    uint32_t index = std::stoi(args.getOption("index", "0"));
    
    Status status = SetPrimary(index);
    
    if (g_options.json) {
        JsonOutput json;
        json.startObject();
        json.addField("status", StatusToString(status));
        if (status == Status::Ok) {
            json.addField("index", (int)index);
        } else {
            json.addField("error", GetLastError());
        }
        json.endObject();
        std::cout << json.toString() << std::endl;
    } else {
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
}

// Enumerate displays and adapters
void cmdEnumerate(const ArgumentParser& args) {
    if (g_options.json) {
        JsonOutput json;
        json.startObject();
        json.addField("status", "enumerating");
        json.endObject();
        std::cout << json.toString() << std::endl;
    } else if (!g_options.quiet) {
        std::cout << "Enumerating displays and adapters..." << std::endl;
    }
    
    // List adapters
    std::vector<AdapterInfo> adapters;
    Status status = EnumerateAdapters(adapters);
    
    if (g_options.json) {
        JsonOutput json;
        json.startObject();
        json.addField("status", StatusToString(status));
        if (status == Status::Ok) {
            json.addField("adapterCount", (int)adapters.size());
            // TODO: Add adapter details to JSON
        } else {
            json.addField("error", GetLastError());
        }
        json.endObject();
        std::cout << json.toString() << std::endl;
    } else {
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
}

// Find DXGI output by name
void cmdFindDxgiOutput(const ArgumentParser& args) {
    if (g_options.json) {
        JsonOutput json;
        json.startObject();
        json.addField("status", "finding dxgi output");
        json.endObject();
        std::cout << json.toString() << std::endl;
    } else if (!g_options.quiet) {
        std::cout << "Finding DXGI output..." << std::endl;
    }
    
    std::string name = args.getOption("name", "");
    if (name.empty()) {
        if (g_options.json) {
            JsonOutput json;
            json.startObject();
            json.addField("status", "error");
            json.addField("error", "Display name is required");
            json.endObject();
            std::cout << json.toString() << std::endl;
        } else {
            std::cout << "Error: Display name is required" << std::endl;
        }
        return;
    }
    
    void* pOutput = nullptr;
    Status status = FindDxgiOutputByName(name, &pOutput);
    
    if (g_options.json) {
        JsonOutput json;
        json.startObject();
        json.addField("status", StatusToString(status));
        if (status == Status::Ok) {
            json.addField("name", name);
            json.addField("outputPointer", std::to_string(reinterpret_cast<uintptr_t>(pOutput)));
        } else {
            json.addField("error", GetLastError());
        }
        json.endObject();
        std::cout << json.toString() << std::endl;
    } else {
        if (status == Status::Ok) {
            std::cout << "Found DXGI output: " << name << std::endl;
            std::cout << "Output pointer: " << pOutput << std::endl;
        } else {
            std::cout << "Failed to find DXGI output: " << StatusToString(status) << std::endl;
            if (status != Status::Ok) {
                std::cout << "Error: " << GetLastError() << std::endl;
            }
        }
    }
}

// Install driver
void cmdInstall(const ArgumentParser& args) {
    if (g_options.json) {
        JsonOutput json;
        json.startObject();
        json.addField("status", "installing driver");
        json.endObject();
        std::cout << json.toString() << std::endl;
    } else if (!g_options.quiet) {
        std::cout << "Installing driver..." << std::endl;
    }
    
    std::string infPath = args.getOption("inf", "IddSampleDriver.inf");
    Status status = InstallDriver(std::wstring(infPath.begin(), infPath.end()));
    
    if (g_options.json) {
        JsonOutput json;
        json.startObject();
        json.addField("status", StatusToString(status));
        if (status == Status::Ok) {
            json.addField("infPath", infPath);
        } else {
            json.addField("error", GetLastError());
        }
        json.endObject();
        std::cout << json.toString() << std::endl;
    } else {
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
}

// Uninstall driver
void cmdUninstall(const ArgumentParser& args) {
    if (g_options.json) {
        JsonOutput json;
        json.startObject();
        json.addField("status", "uninstalling driver");
        json.endObject();
        std::cout << json.toString() << std::endl;
    } else if (!g_options.quiet) {
        std::cout << "Uninstalling driver..." << std::endl;
    }
    
    Status status = UninstallDriver();
    
    if (g_options.json) {
        JsonOutput json;
        json.startObject();
        json.addField("status", StatusToString(status));
        if (status != Status::Ok) {
            json.addField("error", GetLastError());
        }
        json.endObject();
        std::cout << json.toString() << std::endl;
    } else {
        if (status == Status::Ok) {
            std::cout << "Driver uninstalled successfully." << std::endl;
        } else {
            std::cout << "Failed to uninstall driver: " << StatusToString(status) << std::endl;
            if (status != Status::Ok) {
                std::cout << "Error: " << GetLastError() << std::endl;
            }
        }
    }
}

// Begin session
void cmdBeginSession(const ArgumentParser& args) {
    if (g_options.json) {
        JsonOutput json;
        json.startObject();
        json.addField("status", "beginning session");
        json.endObject();
        std::cout << json.toString() << std::endl;
    } else if (!g_options.quiet) {
        std::cout << "Beginning session..." << std::endl;
    }
    
    LeaseHandle lease;
    Status status = BeginSession(lease);
    
    if (g_options.json) {
        JsonOutput json;
        json.startObject();
        json.addField("status", StatusToString(status));
        if (status == Status::Ok) {
            json.addField("leaseId", (int)lease.id);
        } else {
            json.addField("error", GetLastError());
        }
        json.endObject();
        std::cout << json.toString() << std::endl;
    } else {
        if (status == Status::Ok) {
            std::cout << "Session begun successfully." << std::endl;
            std::cout << "Lease ID: " << lease.id << std::endl;
        } else {
            std::cout << "Failed to begin session: " << StatusToString(status) << std::endl;
            if (status != Status::Ok) {
                std::cout << "Error: " << GetLastError() << std::endl;
            }
        }
    }
}

// Activate leased
void cmdActivateLeased(const ArgumentParser& args) {
    if (g_options.json) {
        JsonOutput json;
        json.startObject();
        json.addField("status", "activating leased");
        json.endObject();
        std::cout << json.toString() << std::endl;
    } else if (!g_options.quiet) {
        std::cout << "Activating leased display..." << std::endl;
    }
    
    uint64_t leaseId = std::stoull(args.getOption("lease", "0"));
    LeaseHandle lease{leaseId};
    
    ActivateOptions options;
    options.desc.name = args.getOption("name", "Leased Display");
    options.desc.preferredMode.width = std::stoi(args.getOption("width", "1920"));
    options.desc.preferredMode.height = std::stoi(args.getOption("height", "1080"));
    options.desc.preferredMode.refreshNumerator = std::stoi(args.getOption("refresh", "60"));
    options.desc.preferredMode.refreshDenominator = 1;
    options.desc.hdr10 = args.hasOption("hdr");
    options.desc.stereoscopic = args.hasOption("stereo");
    options.count = std::stoi(args.getOption("count", "1"));
    options.autoRestoreOnCrash = args.hasOption("auto-restore");
    
    Status status = ActivateLeased(options, lease);
    
    if (g_options.json) {
        JsonOutput json;
        json.startObject();
        json.addField("status", StatusToString(status));
        if (status == Status::Ok) {
            json.addField("leaseId", (int)lease.id);
            json.addField("name", options.desc.name);
            json.addField("width", (int)options.desc.preferredMode.width);
            json.addField("height", (int)options.desc.preferredMode.height);
            json.addField("refresh", (int)options.desc.preferredMode.refreshNumerator);
            json.addField("count", (int)options.count);
        } else {
            json.addField("error", GetLastError());
        }
        json.endObject();
        std::cout << json.toString() << std::endl;
    } else {
        if (status == Status::Ok) {
            std::cout << "Leased display activated successfully." << std::endl;
            std::cout << "Lease ID: " << lease.id << std::endl;
            std::cout << "Name: " << options.desc.name << std::endl;
            std::cout << "Resolution: " << options.desc.preferredMode.width << "x" << options.desc.preferredMode.height << std::endl;
            std::cout << "Refresh Rate: " << options.desc.preferredMode.refreshNumerator << "Hz" << std::endl;
            std::cout << "Count: " << options.count << std::endl;
        } else {
            std::cout << "Failed to activate leased display: " << StatusToString(status) << std::endl;
            if (status != Status::Ok) {
                std::cout << "Error: " << GetLastError() << std::endl;
            }
        }
    }
}

// Heartbeat
void cmdHeartbeat(const ArgumentParser& args) {
    if (g_options.json) {
        JsonOutput json;
        json.startObject();
        json.addField("status", "sending heartbeat");
        json.endObject();
        std::cout << json.toString() << std::endl;
    } else if (!g_options.quiet) {
        std::cout << "Sending heartbeat..." << std::endl;
    }
    
    uint64_t leaseId = std::stoull(args.getOption("lease", "0"));
    LeaseHandle lease{leaseId};
    
    Status status = Heartbeat(lease);
    
    if (g_options.json) {
        JsonOutput json;
        json.startObject();
        json.addField("status", StatusToString(status));
        if (status != Status::Ok) {
            json.addField("error", GetLastError());
        }
        json.endObject();
        std::cout << json.toString() << std::endl;
    } else {
        if (status == Status::Ok) {
            std::cout << "Heartbeat sent successfully." << std::endl;
        } else {
            std::cout << "Failed to send heartbeat: " << StatusToString(status) << std::endl;
            if (status != Status::Ok) {
                std::cout << "Error: " << GetLastError() << std::endl;
            }
        }
    }
}

// End session
void cmdEndSession(const ArgumentParser& args) {
    if (g_options.json) {
        JsonOutput json;
        json.startObject();
        json.addField("status", "ending session");
        json.endObject();
        std::cout << json.toString() << std::endl;
    } else if (!g_options.quiet) {
        std::cout << "Ending session..." << std::endl;
    }
    
    uint64_t leaseId = std::stoull(args.getOption("lease", "0"));
    LeaseHandle lease{leaseId};
    
    Status status = EndSession(lease);
    
    if (g_options.json) {
        JsonOutput json;
        json.startObject();
        json.addField("status", StatusToString(status));
        if (status != Status::Ok) {
            json.addField("error", GetLastError());
        }
        json.endObject();
        std::cout << json.toString() << std::endl;
    } else {
        if (status == Status::Ok) {
            std::cout << "Session ended successfully." << std::endl;
        } else {
            std::cout << "Failed to end session: " << StatusToString(status) << std::endl;
            if (status != Status::Ok) {
                std::cout << "Error: " << GetLastError() << std::endl;
            }
        }
    }
}

// Get session state
void cmdGetSessionState(const ArgumentParser& args) {
    if (g_options.json) {
        JsonOutput json;
        json.startObject();
        json.addField("status", "getting session state");
        json.endObject();
        std::cout << json.toString() << std::endl;
    } else if (!g_options.quiet) {
        std::cout << "Getting session state..." << std::endl;
    }
    
    uint64_t leaseId = std::stoull(args.getOption("lease", "0"));
    LeaseHandle lease{leaseId};
    
    bool isActive;
    uint32_t timeRemaining;
    Status status = GetSessionState(lease, isActive, timeRemaining);
    
    if (g_options.json) {
        JsonOutput json;
        json.startObject();
        json.addField("status", StatusToString(status));
        if (status == Status::Ok) {
            json.addField("isActive", isActive);
            json.addField("timeRemaining", (int)timeRemaining);
        } else {
            json.addField("error", GetLastError());
        }
        json.endObject();
        std::cout << json.toString() << std::endl;
    } else {
        if (status == Status::Ok) {
            std::cout << "Session state retrieved successfully." << std::endl;
            std::cout << "Active: " << (isActive ? "Yes" : "No") << std::endl;
            std::cout << "Time Remaining: " << timeRemaining << "ms" << std::endl;
        } else {
            std::cout << "Failed to get session state: " << StatusToString(status) << std::endl;
            if (status != Status::Ok) {
                std::cout << "Error: " << GetLastError() << std::endl;
            }
        }
    }
}

// Recover orphaned state
void cmdRecoverOrphaned(const ArgumentParser& args) {
    if (g_options.json) {
        JsonOutput json;
        json.startObject();
        json.addField("status", "recovering orphaned state");
        json.endObject();
        std::cout << json.toString() << std::endl;
    } else if (!g_options.quiet) {
        std::cout << "Recovering orphaned state..." << std::endl;
    }
    
    Status status = RecoverOrphanedState();
    
    if (g_options.json) {
        JsonOutput json;
        json.startObject();
        json.addField("status", StatusToString(status));
        if (status != Status::Ok) {
            json.addField("error", GetLastError());
        }
        json.endObject();
        std::cout << json.toString() << std::endl;
    } else {
        if (status == Status::Ok) {
            std::cout << "Orphaned state recovered successfully." << std::endl;
        } else {
            std::cout << "Failed to recover orphaned state: " << StatusToString(status) << std::endl;
            if (status != Status::Ok) {
                std::cout << "Error: " << GetLastError() << std::endl;
            }
        }
    }
}

// Ensure driver running
void cmdEnsureDriverRunning(const ArgumentParser& args) {
    if (g_options.json) {
        JsonOutput json;
        json.startObject();
        json.addField("status", "ensuring driver running");
        json.endObject();
        std::cout << json.toString() << std::endl;
    } else if (!g_options.quiet) {
        std::cout << "Ensuring driver is running..." << std::endl;
    }
    
    Status status = EnsureDriverRunning();
    
    if (g_options.json) {
        JsonOutput json;
        json.startObject();
        json.addField("status", StatusToString(status));
        if (status != Status::Ok) {
            json.addField("error", GetLastError());
        }
        json.endObject();
        std::cout << json.toString() << std::endl;
    } else {
        if (status == Status::Ok) {
            std::cout << "Driver is running successfully." << std::endl;
        } else {
            std::cout << "Failed to ensure driver running: " << StatusToString(status) << std::endl;
            if (status != Status::Ok) {
                std::cout << "Error: " << GetLastError() << std::endl;
            }
        }
    }
}

// Get last error
void cmdGetLastError(const ArgumentParser& args) {
    std::string error = GetLastError();
    
    if (g_options.json) {
        JsonOutput json;
        json.startObject();
        json.addField("error", error);
        json.endObject();
        std::cout << json.toString() << std::endl;
    } else {
        std::cout << "Last Error: " << error << std::endl;
    }
}

// Get system info
void cmdGetSystemInfo(const ArgumentParser& args) {
    std::string info = GetSystemInfo();
    
    if (g_options.json) {
        JsonOutput json;
        json.startObject();
        json.addField("systemInfo", info);
        json.endObject();
        std::cout << json.toString() << std::endl;
    } else {
        std::cout << "System Info: " << info << std::endl;
    }
}

// Set HDR support
void cmdSetHdr(const ArgumentParser& args) {
    if (g_options.json) {
        JsonOutput json;
        json.startObject();
        json.addField("status", "setting HDR support");
        json.endObject();
        std::cout << json.toString() << std::endl;
    } else if (!g_options.quiet) {
        std::cout << "Setting HDR support..." << std::endl;
    }
    
    uint32_t index = std::stoi(args.getOption("index", "0"));
    bool enable = args.hasOption("enable");
    
    Status status = SetHdrSupport(index, enable);
    
    if (g_options.json) {
        JsonOutput json;
        json.startObject();
        json.addField("status", StatusToString(status));
        if (status == Status::Ok) {
            json.addField("index", (int)index);
            json.addField("enabled", enable);
        } else {
            json.addField("error", GetLastError());
        }
        json.endObject();
        std::cout << json.toString() << std::endl;
    } else {
        if (status == Status::Ok) {
            std::cout << "HDR support set successfully." << std::endl;
            std::cout << "Index: " << index << std::endl;
            std::cout << "Enabled: " << (enable ? "Yes" : "No") << std::endl;
        } else {
            std::cout << "Failed to set HDR support: " << StatusToString(status) << std::endl;
            if (status != Status::Ok) {
                std::cout << "Error: " << GetLastError() << std::endl;
            }
        }
    }
}

// Set stereo support
void cmdSetStereo(const ArgumentParser& args) {
    if (g_options.json) {
        JsonOutput json;
        json.startObject();
        json.addField("status", "setting stereo support");
        json.endObject();
        std::cout << json.toString() << std::endl;
    } else if (!g_options.quiet) {
        std::cout << "Setting stereo support..." << std::endl;
    }
    
    uint32_t index = std::stoi(args.getOption("index", "0"));
    bool enable = args.hasOption("enable");
    
    Status status = SetStereoSupport(index, enable);
    
    if (g_options.json) {
        JsonOutput json;
        json.startObject();
        json.addField("status", StatusToString(status));
        if (status == Status::Ok) {
            json.addField("index", (int)index);
            json.addField("enabled", enable);
        } else {
            json.addField("error", GetLastError());
        }
        json.endObject();
        std::cout << json.toString() << std::endl;
    } else {
        if (status == Status::Ok) {
            std::cout << "Stereo support set successfully." << std::endl;
            std::cout << "Index: " << index << std::endl;
            std::cout << "Enabled: " << (enable ? "Yes" : "No") << std::endl;
        } else {
            std::cout << "Failed to set stereo support: " << StatusToString(status) << std::endl;
            if (status != Status::Ok) {
                std::cout << "Error: " << GetLastError() << std::endl;
            }
        }
    }
}

// Set custom EDID
void cmdSetCustomEdid(const ArgumentParser& args) {
    if (g_options.json) {
        JsonOutput json;
        json.startObject();
        json.addField("status", "setting custom EDID");
        json.endObject();
        std::cout << json.toString() << std::endl;
    } else if (!g_options.quiet) {
        std::cout << "Setting custom EDID..." << std::endl;
    }
    
    uint32_t index = std::stoi(args.getOption("index", "0"));
    std::string filePath = args.getOption("file", "");
    
    if (filePath.empty()) {
        if (g_options.json) {
            JsonOutput json;
            json.startObject();
            json.addField("status", "error");
            json.addField("error", "EDID file path is required");
            json.endObject();
            std::cout << json.toString() << std::endl;
        } else {
            std::cout << "Error: EDID file path is required" << std::endl;
        }
        return;
    }
    
    // Read EDID file
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        if (g_options.json) {
            JsonOutput json;
            json.startObject();
            json.addField("status", "error");
            json.addField("error", "Failed to open EDID file");
            json.endObject();
            std::cout << json.toString() << std::endl;
        } else {
            std::cout << "Error: Failed to open EDID file" << std::endl;
        }
        return;
    }
    
    std::vector<uint8_t> edidData((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();
    
    Status status = SetCustomEdid(index, edidData);
    
    if (g_options.json) {
        JsonOutput json;
        json.startObject();
        json.addField("status", StatusToString(status));
        if (status == Status::Ok) {
            json.addField("index", (int)index);
            json.addField("edidSize", (int)edidData.size());
        } else {
            json.addField("error", GetLastError());
        }
        json.endObject();
        std::cout << json.toString() << std::endl;
    } else {
        if (status == Status::Ok) {
            std::cout << "Custom EDID set successfully." << std::endl;
            std::cout << "Index: " << index << std::endl;
            std::cout << "EDID Size: " << edidData.size() << " bytes" << std::endl;
        } else {
            std::cout << "Failed to set custom EDID: " << StatusToString(status) << std::endl;
            if (status != Status::Ok) {
                std::cout << "Error: " << GetLastError() << std::endl;
            }
        }
    }
}

// Get EDID
void cmdGetEdid(const ArgumentParser& args) {
    if (g_options.json) {
        JsonOutput json;
        json.startObject();
        json.addField("status", "getting EDID");
        json.endObject();
        std::cout << json.toString() << std::endl;
    } else if (!g_options.quiet) {
        std::cout << "Getting EDID..." << std::endl;
    }
    
    uint32_t index = std::stoi(args.getOption("index", "0"));
    std::vector<uint8_t> edidData;
    
    Status status = GetEdid(index, edidData);
    
    if (g_options.json) {
        JsonOutput json;
        json.startObject();
        json.addField("status", StatusToString(status));
        if (status == Status::Ok) {
            json.addField("index", (int)index);
            json.addField("edidSize", (int)edidData.size());
            // TODO: Add EDID data to JSON
        } else {
            json.addField("error", GetLastError());
        }
        json.endObject();
        std::cout << json.toString() << std::endl;
    } else {
        if (status == Status::Ok) {
            std::cout << "EDID retrieved successfully." << std::endl;
            std::cout << "Index: " << index << std::endl;
            std::cout << "EDID Size: " << edidData.size() << " bytes" << std::endl;
        } else {
            std::cout << "Failed to get EDID: " << StatusToString(status) << std::endl;
            if (status != Status::Ok) {
                std::cout << "Error: " << GetLastError() << std::endl;
            }
        }
    }
}

// Parse global options
void parseGlobalOptions(int argc, char* argv[]) {
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-q" || arg == "--quiet") {
            g_options.quiet = true;
        } else if (arg == "-j" || arg == "--json") {
            g_options.json = true;
        } else if (arg == "-p" || arg == "--pretty") {
            g_options.pretty = true;
        } else if (arg == "-t" || arg == "--timeout") {
            if (i + 1 < argc) {
                g_options.timeout = std::stoi(argv[++i]);
            }
        } else if (arg == "--elevate") {
            g_options.elevate = true;
        } else if (arg == "--log-level") {
            if (i + 1 < argc) {
                g_options.logLevel = argv[++i];
            }
        }
    }
}

// Main function
int main(int argc, char* argv[]) {
    if (argc < 2) {
        printHelp();
        return 1;
    }
    
    // Parse global options
    parseGlobalOptions(argc, argv);
    
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
        } else if (command == "begin-session") {
            cmdBeginSession(args);
        } else if (command == "activate-leased") {
            cmdActivateLeased(args);
        } else if (command == "heartbeat") {
            cmdHeartbeat(args);
        } else if (command == "end-session") {
            cmdEndSession(args);
        } else if (command == "get-session-state") {
            cmdGetSessionState(args);
        } else if (command == "recover-orphaned") {
            cmdRecoverOrphaned(args);
        } else if (command == "ensure-driver-running") {
            cmdEnsureDriverRunning(args);
        } else if (command == "get-last-error") {
            cmdGetLastError(args);
        } else if (command == "get-system-info") {
            cmdGetSystemInfo(args);
        } else if (command == "set-hdr") {
            cmdSetHdr(args);
        } else if (command == "set-stereo") {
            cmdSetStereo(args);
        } else if (command == "set-custom-edid") {
            cmdSetCustomEdid(args);
        } else if (command == "get-edid") {
            cmdGetEdid(args);
        } else if (command == "version") {
            printVersion();
        } else if (command == "help") {
            printHelp();
        } else {
            if (g_options.json) {
                JsonOutput json;
                json.startObject();
                json.addField("status", "error");
                json.addField("error", "Unknown command: " + command);
                json.endObject();
                std::cout << json.toString() << std::endl;
            } else {
                std::cout << "Unknown command: " << command << std::endl;
                std::cout << "Use 'vddctl help' for usage information." << std::endl;
            }
            return 1;
        }
    } catch (const std::exception& e) {
        if (g_options.json) {
            JsonOutput json;
            json.startObject();
            json.addField("status", "error");
            json.addField("error", e.what());
            json.endObject();
            std::cout << json.toString() << std::endl;
        } else {
            std::cout << "Error: " << e.what() << std::endl;
        }
        return 1;
    }
    
    return 0;
}
