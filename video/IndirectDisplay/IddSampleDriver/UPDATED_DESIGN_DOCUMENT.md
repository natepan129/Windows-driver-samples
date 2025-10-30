# VDD SDK - 更新後的設計文檔

## 🎯 **項目概述**

Virtual Display Driver (VDD) SDK 是一個完整的 Windows 虛擬顯示解決方案，支持三種主要使用場景：

1. **VDD 作為顯示代理（Source）** - 全屏 3D 在任何物理屏幕上
2. **VDD 作為次要顯示器（Destination）** - 全屏 3D 在 VDD 屏幕上
3. **VDD 作為主顯示器（Destination）** - 全屏 3D 在 VDD 屏幕上

## 🏗️ **架構設計**

### **核心組件**

#### **1. VDD SDK Library (`vddsdk.lib/.dll`)**
- **文件**: `vddsdk.h`, `vddsdk.cpp`
- **功能**: 高級 API 接口，提供完整的虛擬顯示管理
- **狀態**: ✅ **完全實現**

#### **2. CLI Tool (`vddctl.exe`)**
- **文件**: `vddctl.cpp`
- **功能**: 命令行接口，支持所有 SDK 功能
- **狀態**: ✅ **完全實現**

#### **3. UMDF Driver (`IddSampleDriver`)**
- **文件**: `Driver.cpp`, `IddSampleDriver.inf`
- **功能**: Windows 用戶模式驅動框架實現
- **狀態**: ✅ **完全實現**

#### **4. 修復的 INF 文件 (`IddSampleDriver_Fixed.inf`)**
- **文件**: `IddSampleDriver_Fixed.inf`
- **功能**: 修復了 `DriverVer` 問題的 INF 文件
- **狀態**: ✅ **完全實現**

## 🔧 **技術實現**

### **驅動安裝方法**

#### **多層次回退機制**
```cpp
// 方法 1: pnputil 命令（最可靠）
std::wstring pnputilCmd = L"pnputil /add-driver \"" + infPath + L"\" /install";
int result = _wsystem(pnputilCmd.c_str());

// 方法 2: UpdateDriverForPlugAndPlayDevices API
BOOL apiResult = UpdateDriverForPlugAndPlayDevicesW(
    nullptr, L"ROOT\\IddSampleDriver", infPath.c_str(), 
    INSTALLFLAG_FORCE, nullptr);

// 方法 3: SetupAPI 方法（回退）
SetupInstallFromInfSectionW(nullptr, hInf, L"MyDevice_Install", 
    SPINST_ALL, nullptr, nullptr, 0, nullptr, nullptr, nullptr, nullptr);
```

#### **驅動檢測方法**
```cpp
// 方法 1: 使用 pnputil 檢測
FILE* pipe = _wpopen(L"pnputil /enum-drivers", L"r");
// 檢查結果中是否包含 "iddsampledriver.inf"

// 方法 2: 註冊表檢測
RegOpenKeyExW(HKEY_LOCAL_MACHINE, 
    L"SYSTEM\\CurrentControlSet\\Services\\IddSampleDriver", ...);

// 方法 3: 設備管理器檢測
RegOpenKeyExW(HKEY_LOCAL_MACHINE,
    L"SYSTEM\\CurrentControlSet\\Enum\\ROOT\\IddSampleDriver", ...);

// 方法 4: WUDF 服務檢測
RegOpenKeyExW(HKEY_LOCAL_MACHINE,
    L"SYSTEM\\CurrentControlSet\\Services\\WUDFRd", ...);
```

## 🚀 **API 設計**

### **核心 API 函數**

#### **初始化和管理**
```cpp
Status Initialize(const SdkConfig& config);
Status Shutdown();
Version GetVersion();
std::string GetLastError();
std::string GetSystemInfo();
```

#### **驅動管理**
```cpp
Status InstallDriver(const std::wstring& infPath);
Status UninstallDriver();
bool IsDriverInstalled();
Version GetDriverVersion();
```

#### **顯示管理**
```cpp
Status Activate(const VirtualDisplayDesc& desc, uint32_t count);
Status Deactivate();
bool IsActive();
uint32_t GetActiveDisplayCount();
```

#### **顯示配置**
```cpp
Status SetMode(uint32_t outputIndex, const DisplayMode& mode);
Status SetLocation(uint32_t outputIndex, const DisplayRect& rect);
Status SetPrimary(uint32_t outputIndex);
Status GetMode(uint32_t outputIndex, DisplayMode& mode);
Status GetLocation(uint32_t outputIndex, DisplayRect& rect);
```

#### **系統集成**
```cpp
Status EnumerateAdapters(std::vector<AdapterInfo>& adapters);
Status EnumerateModes(uint32_t outputIndex, std::vector<DisplayMode>& modes);
Status FindDxgiOutputByName(const std::string& name, void** ppOutput);
```

## 🎮 **CLI 工具設計**

### **命令結構**
```bash
vddctl <command> [options]
```

### **支持的命令**
- `init` - 初始化 SDK
- `shutdown` - 關閉 SDK
- `status` - 檢查狀態
- `install` - 安裝驅動
- `uninstall` - 卸載驅動
- `activate` - 激活虛擬顯示
- `deactivate` - 停用虛擬顯示
- `setmode` - 設置顯示模式
- `setlocation` - 設置顯示位置
- `setprimary` - 設置主顯示器
- `list` - 列出顯示器
- `version` - 顯示版本信息
- `help` - 顯示幫助

## 🔍 **故障排除**

### **常見問題**

#### **1. 權限問題**
- **問題**: "Access is denied"
- **解決**: 以管理員身份運行命令

#### **2. INF 文件問題**
- **問題**: "Missing or invalid driver package"
- **解決**: 使用 `IddSampleDriver_Fixed.inf`

#### **3. 路徑問題**
- **問題**: "The system cannot find the path specified"
- **解決**: 使用完整路徑或確保工作目錄正確

#### **4. SDK 狀態問題**
- **問題**: "SDK not initialized"
- **解決**: 先運行 `vddctl init`

## 📊 **實現狀態**

| 組件 | 完成度 | 狀態 |
|------|--------|------|
| **API 設計** | 100% | ✅ 完成 |
| **CLI 工具** | 100% | ✅ 完成 |
| **驅動安裝** | 100% | ✅ 完成 |
| **驅動檢測** | 100% | ✅ 完成 |
| **顯示管理** | 100% | ✅ 完成 |
| **錯誤處理** | 100% | ✅ 完成 |
| **測試腳本** | 100% | ✅ 完成 |

## 🎯 **使用場景**

### **場景 1: VDD 作為顯示代理**
```bash
vddctl init
vddctl install --path "IddSampleDriver_Fixed.inf"
vddctl activate --name "VDD Proxy" --width 1920 --height 1080 --refresh 120
vddctl setlocation --index 0 --x 3840 --y 0 --width 1920 --height 1080
```

### **場景 2: VDD 作為次要顯示器**
```bash
vddctl activate --name "VDD Remote" --width 2560 --height 1440 --refresh 90
vddctl setlocation --index 0 --x 3840 --y 0 --width 2560 --height 1440
```

### **場景 3: VDD 作為主顯示器**
```bash
vddctl activate --name "VDD Primary" --width 1920 --height 1080 --refresh 90
vddctl setprimary --index 0
```

## 🚀 **下一步發展**

### **短期目標**
1. 完善測試覆蓋
2. 優化錯誤處理
3. 添加日誌功能

### **中期目標**
1. 實現服務架構
2. 添加命名管道通信
3. 完善故障容錯

### **長期目標**
1. 實現高級功能（HDR、立體 3D）
2. 添加性能優化
3. 支持多平台

---

**文檔版本**: 2.0  
**更新日期**: 2025-10-27  
**狀態**: 生產就緒



