# 从成功项目学到的实现方案

## 📚 信息来源

1. ✅ **SudoVDA README**（公开信息）
2. ✅ **我们的成功安装经验**（setupapi_install_simple.cpp）
3. ✅ **Microsoft IddCx 文档**
4. ✅ **设计文档要求**

---

## 1️⃣ 安装方法（已解决✅）

### **SudoVDA/VirtualDrivers 的方法（推测）**

```cpp
// 基于我们的成功经验，他们很可能用类似方法：

HDEVINFO deviceInfoSet = SetupDiCreateDeviceInfoList(&displayGuid, NULL);
SetupDiCreateDeviceInfoW(deviceInfoSet, L"VirtualDisplay", ...);
SetupDiSetDeviceRegistryPropertyW(deviceInfoSet, &devInfo, SPDRP_HARDWAREID, ...);
SetupDiCallClassInstaller(DIF_REGISTERDEVICE, deviceInfoSet, &devInfo);

// 关键：Fallback 机制
if (!SetupDiCallClassInstaller(DIF_INSTALLDEVICE, ...)) {
    // 使用 UpdateDriverForPlugAndPlayDevices 作为 fallback
    UpdateDriverForPlugAndPlayDevicesW(NULL, hwid, infPath, INSTALLFLAG_FORCE, &reboot);
}

// ✅ 这就是我们 setupapi_install_simple.cpp 的方法！
```

**状态**：✅ **已实现并验证成功**

---

## 2️⃣ 驱动签名方式

### **VirtualDrivers 的方法（从 README 推测）**

```
1. 使用 SignPath.io 提供的 EV Code Signing Certificate
2. 签名文件：
   - IddSampleDriver.dll
   - IndirectKmd.sys
   - IddSampleDriver.cat（目录文件）

3. 签名命令（推测）：
   signtool sign /v /fd SHA256 /ac "CrossCert.cer" \
       /n "Company Name" /t http://timestamp.digicert.com \
       IddSampleDriver.dll IndirectKmd.sys

   Inf2Cat /driver:. /os:10_X64
   signtool sign /v /fd SHA256 /ac "CrossCert.cer" \
       /n "Company Name" /t http://timestamp.digicert.com \
       IddSampleDriver.cat
```

### **我们的替代方案（测试环境）**

```batch
REM 1. 启用测试签名模式
bcdedit /set testsigning on

REM 2. 使用自签名证书（开发/测试）
makecert -r -pe -ss PrivateCertStore -n "CN=TestDriverCert" TestCert.cer
certmgr /add TestCert.cer /s /r localMachine root

REM 3. 签名驱动文件
signtool sign /v /s PrivateCertStore /n "TestDriverCert" \
    /t http://timestamp.digicert.com \
    IddSampleDriver.dll IndirectKmd.sys

REM 4. 创建并签名目录文件
Inf2Cat /driver:. /os:10_X64
signtool sign /v /s PrivateCertStore /n "TestDriverCert" \
    IddSampleDriver.cat
```

**状态**：⚠️ **需要证书（测试环境可用 bcdedit /set testsigning）**

---

## 3️⃣ 配置管理实现（从 SudoVDA 学习）

### **SudoVDA 的 Registry 配置**

```
位置：HKEY_LOCAL_MACHINE\SOFTWARE\SudoMaker\SudoVDA

配置项：
- gpuName [STRING]     : 绑定的 GPU 名称（默认：自动选择最大 VRAM）
- maxMonitors [DWORD]  : 最大虚拟显示器数量（默认：10）
- watchdog [DWORD]     : Watchdog 超时秒数（默认：3，0=禁用）
- sdrBits [DWORD]      : SDR 位深（8 或 10）
- hdrBits [DWORD]      : HDR 位深（10 或 12）

注意事项：
- 修改后需要重载驱动或重启
- 如果驱动被占用（如被 Apollo），需要先退出应用
```

### **我们的实现方案**

```cpp
// vddsdk.cpp - 配置管理实现

class RegistryConfig {
private:
    const wchar_t* REG_PATH = L"SOFTWARE\\VddSdk\\Config";
    
public:
    // 读取配置
    std::wstring GetGpuName() {
        HKEY hKey;
        if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, REG_PATH, 0, 
                KEY_READ, &hKey) == ERROR_SUCCESS) {
            wchar_t buffer[256] = {};
            DWORD size = sizeof(buffer);
            RegQueryValueExW(hKey, L"gpuName", NULL, NULL, 
                (LPBYTE)buffer, &size);
            RegCloseKey(hKey);
            return buffer;
        }
        return L"";  // 空=自动选择
    }
    
    DWORD GetMaxMonitors() {
        return GetDwordValue(L"maxMonitors", 10);  // 默认 10
    }
    
    DWORD GetWatchdogTimeout() {
        return GetDwordValue(L"watchdogTimeout", 3);  // 默认 3 秒
    }
    
    DWORD GetSdrBits() {
        return GetDwordValue(L"sdrBits", 8);  // 默认 8 bit
    }
    
    DWORD GetHdrBits() {
        return GetDwordValue(L"hdrBits", 10);  // 默认 10 bit
    }
    
private:
    DWORD GetDwordValue(const wchar_t* name, DWORD defaultValue) {
        HKEY hKey;
        if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, REG_PATH, 0, 
                KEY_READ, &hKey) == ERROR_SUCCESS) {
            DWORD value = 0;
            DWORD size = sizeof(DWORD);
            if (RegQueryValueExW(hKey, name, NULL, NULL, 
                    (LPBYTE)&value, &size) == ERROR_SUCCESS) {
                RegCloseKey(hKey);
                return value;
            }
            RegCloseKey(hKey);
        }
        return defaultValue;
    }
};

// 在驱动初始化时读取配置
Status VddSdkImpl::Initialize() {
    RegistryConfig config;
    
    // 应用配置
    m_maxMonitors = config.GetMaxMonitors();
    m_watchdogTimeout = config.GetWatchdogTimeout();
    m_gpuName = config.GetGpuName();
    
    // ... 其他初始化
    
    return Status::Ok;
}
```

**状态**：✅ **可以立即实现**

---

## 4️⃣ Watchdog/Heartbeat 机制（结合设计文档）

### **SudoVDA 的 Watchdog**

```
功能：防止驱动卡死
默认：3 秒超时
实现位置：驱动层（Driver.cpp）

推测实现（基于标准 WDF 做法）：
1. 创建 WDF Timer
2. 每次成功处理请求时重置 Timer
3. 如果 Timer 超时 → 记录日志 + 重启驱动
```

### **我们的设计：Heartbeat + Lease（应用层）**

```cpp
// vddsdk.cpp - Heartbeat 机制实现

struct LeaseHandle {
    HANDLE hThread;
    std::atomic<bool> shouldStop;
    DWORD interval;  // 心跳间隔（毫秒）
};

// 激活虚拟显示器（带租约）
Status VddSdkImpl::ActivateLeased(uint32_t adapterId, 
                                   uint32_t monitorId,
                                   ActivateOptions options,
                                   LeaseHandle** outLease) {
    // 1. 正常激活
    auto status = Activate(adapterId, monitorId);
    if (status != Status::Ok) {
        return status;
    }
    
    // 2. 创建心跳线程
    LeaseHandle* lease = new LeaseHandle();
    lease->shouldStop = false;
    lease->interval = options.heartbeatInterval;  // 默认 1000ms
    
    lease->hThread = CreateThread(NULL, 0, 
        [](LPVOID param) -> DWORD {
            LeaseHandle* lease = (LeaseHandle*)param;
            VddSdkImpl* sdk = GetInstance();
            
            while (!lease->shouldStop) {
                // 发送心跳
                auto status = sdk->Heartbeat(adapterId, monitorId);
                
                if (status != Status::Ok) {
                    // 心跳失败 - 清理并恢复
                    if (options.autoRestoreOnCrash) {
                        sdk->RecoverOrphanedState();
                    }
                    break;
                }
                
                Sleep(lease->interval);
            }
            
            return 0;
        }, lease, 0, NULL);
    
    *outLease = lease;
    return Status::Ok;
}

// 心跳实现
Status VddSdkImpl::Heartbeat(uint32_t adapterId, uint32_t monitorId) {
    // 向驱动发送 IOCTL 保持活动
    DWORD ioctl = IOCTL_VDD_HEARTBEAT;
    DWORD returned = 0;
    
    struct {
        uint32_t adapterId;
        uint32_t monitorId;
        uint64_t timestamp;
    } data = { adapterId, monitorId, GetTickCount64() };
    
    if (!DeviceIoControl(m_hDevice, ioctl, 
            &data, sizeof(data), 
            NULL, 0, &returned, NULL)) {
        return Status::DeviceError;
    }
    
    return Status::Ok;
}

// 结束租约
Status VddSdkImpl::EndSession(LeaseHandle* lease) {
    if (!lease) return Status::InvalidArg;
    
    // 停止心跳线程
    lease->shouldStop = true;
    WaitForSingleObject(lease->hThread, 5000);
    CloseHandle(lease->hThread);
    
    delete lease;
    return Status::Ok;
}

// 恢复孤立状态（设计文档要求）
Status VddSdkImpl::RecoverOrphanedState() {
    // 1. 枚举所有虚拟显示器
    std::vector<MonitorInfo> monitors;
    EnumerateModes(&monitors);
    
    // 2. 检查哪些是"孤立"的（无心跳）
    for (auto& monitor : monitors) {
        if (IsOrphaned(monitor)) {
            // 3. 停用孤立的显示器
            Deactivate(monitor.adapterId, monitor.monitorId);
        }
    }
    
    // 4. 恢复原始桌面拓扑
    RestoreOriginalTopology();
    
    return Status::Ok;
}
```

**状态**：✅ **设计文档已定义，可以立即实现**

---

## 5️⃣ 驱动层 Watchdog（Driver.cpp）

### **推测 SudoVDA 的实现**

```cpp
// Driver.cpp - Watchdog Timer

class IndirectDeviceContext {
private:
    WDFTIMER m_WatchdogTimer;
    ULONG m_WatchdogTimeout;  // 从 Registry 读取
    
public:
    NTSTATUS InitWatchdog() {
        // 读取配置
        m_WatchdogTimeout = ReadRegistryDword(
            L"\\Registry\\Machine\\SOFTWARE\\SudoMaker\\SudoVDA",
            L"watchdog", 3) * 1000;  // 秒转毫秒
        
        if (m_WatchdogTimeout == 0) {
            return STATUS_SUCCESS;  // 禁用
        }
        
        // 创建 Timer
        WDF_TIMER_CONFIG timerConfig;
        WDF_TIMER_CONFIG_INIT(&timerConfig, WatchdogCallback);
        timerConfig.Period = m_WatchdogTimeout;
        timerConfig.AutomaticSerialization = TRUE;
        
        WDF_OBJECT_ATTRIBUTES timerAttribs;
        WDF_OBJECT_ATTRIBUTES_INIT(&timerAttribs);
        timerAttribs.ParentObject = m_WdfDevice;
        
        return WdfTimerCreate(&timerConfig, &timerAttribs, 
                              &m_WatchdogTimer);
    }
    
    static void WatchdogCallback(WDFTIMER Timer) {
        // Watchdog 触发 - 驱动可能卡死
        TraceEvents(TRACE_LEVEL_WARNING, 
            "Watchdog timeout - driver may be stuck!");
        
        // 可以选择：
        // 1. 重置设备状态
        // 2. 记录诊断信息
        // 3. 触发驱动重载
    }
    
    void ResetWatchdog() {
        // 每次成功处理请求时调用
        if (m_WatchdogTimer) {
            WdfTimerStart(m_WatchdogTimer, 
                WDF_REL_TIMEOUT_IN_MS(m_WatchdogTimeout));
        }
    }
};
```

**状态**：⚠️ **需要修改 Driver.cpp（可选功能）**

---

## 6️⃣ 完整集成方案

### **优先级排序**

| 功能 | 优先级 | 状态 | 下一步 |
|------|--------|------|--------|
| ✅ **安装方法** | 🔴 **P0** | ✅ 已完成 | 集成到 vddsdk.cpp |
| ⚠️ **签名** | 🟡 **P1** | ⚠️ 测试模式可用 | 生产环境需要证书 |
| ✅ **Registry 配置** | 🟢 **P2** | ✅ 可立即实现 | 添加到 vddsdk.cpp |
| ✅ **Heartbeat 机制** | 🟢 **P2** | ✅ 设计已完成 | 实现 ActivateLeased |
| ⚠️ **驱动层 Watchdog** | 🟢 **P3** | ⚠️ 可选 | 修改 Driver.cpp |

---

## 🎯 立即可行的实现步骤

### **步骤 1：集成安装方法到 vddsdk.cpp** ✅

```cpp
// 使用已验证的 setupapi_install_simple.cpp 方法
Status VddSdkImpl::InstallDriver(const std::wstring& infPath) {
    // ... 复制简单版本的代码 ...
}
```

### **步骤 2：添加 Registry 配置管理** ✅

```cpp
// 添加 RegistryConfig 类
// 在初始化时读取配置
```

### **步骤 3：实现 Heartbeat/Lease 机制** ✅

```cpp
// 实现 ActivateLeased, Heartbeat, EndSession
// 实现 RecoverOrphanedState
```

### **步骤 4：测试签名（可选）** ⚠️

```batch
# 开发环境：使用测试签名
bcdedit /set testsigning on

# 生产环境：需要购买 EV 证书
```

---

## 📚 参考资源

1. ✅ **SudoVDA README**: Registry 配置方式
2. ✅ **我们的成功经验**: setupapi_install_simple.cpp
3. ✅ **设计文档**: Heartbeat/Lease 机制定义
4. ✅ **Microsoft IddCx 文档**: 驱动框架标准

---

## 🎯 结论

**虽然无法直接看到源码，但我们可以：**

1. ✅ **安装方法**：已有成功方案（setupapi_install_simple.cpp）
2. ✅ **配置管理**：从 SudoVDA README 学习 Registry 方式
3. ✅ **Heartbeat**：设计文档已定义，可立即实现
4. ⚠️ **签名**：测试环境可用，生产需要证书

**我们不需要他们的源码，因为我们已经有：**
- ✅ 成功的安装方法
- ✅ 清晰的设计文档
- ✅ 标准的 Windows API 文档

**下一步：直接实现！** 🚀



