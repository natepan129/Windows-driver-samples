# VDD 驱动安装整合路线图

## 🎯 选择的方案：**方案 B + 方案 A 部分理念**

**核心策略**：在 `vddctl` 内直接使用 SetupAPI，加入完整的安全机制和回滚逻辑。

---

## 📊 方案对比

| 方案 | 优点 | 缺点 | 实施难度 | 推荐度 |
|------|------|------|---------|--------|
| **A: 外部 EXE 封装** | 快速上线、保留现有能力 | 黑箱多、难以精确控制 | ⭐ 低 | ⭐⭐ |
| **B: SetupAPI 直接集成** | 完全可控、可事务回滚、防护完整 | 开发量较大 | ⭐⭐ 中 | ⭐⭐⭐⭐⭐ |
| **C: WiX Burn** | 产品化、专业 | 开发周期长、过度工程 | ⭐⭐⭐ 高 | ⭐⭐⭐ (长期) |

**选择 B 的原因**：
- ✅ 我们已有 SetupAPI 基础代码（`vddsdk.cpp`）
- ✅ 已验证可用的 `install_driver.exe` / `uninstall_driver.exe`
- ✅ 提供最大控制权和安全性
- ✅ 符合设计文档要求

---

## 🚀 实施路线图

### 阶段 1: 基础功能（1-2 天）✅ 部分完成

**目标**：让 `vddctl` 可以安装和卸载驱动

**任务**：
- [x] 实现 `vddsdk.cpp` 的 `InstallDriver()` - SetupAPI 方法
- [x] 实现 `vddsdk.cpp` 的 `UninstallDriver()` - SetupAPI 方法
- [x] 创建 `vddctl.cpp` 命令行工具
- [x] 修复 `vddctl install --inf` 参数解析 bug
- [ ] 编译 `vddctl.exe`（当前阻塞：需要修复编译配置）
- [ ] 测试基本安装/卸载流程

**当前状态**：
- ✅ 代码已实现
- ⚠️ 编译配置需要修复
- ⚠️ 需要完整测试

---

### 阶段 2: 安全机制（3-5 天）🔄 待实施

**目标**：防止黑屏、SID 问题、鼠标偏移等风险

#### 2.1 卸载前的安全检查

**实现位置**：`vddsdk.cpp::UninstallDriver()`

```cpp
Status VddSdkImpl::UninstallDriver(UninstallOptions options) {
    // 1. 检查当前主显示器
    if (IsPrimaryDisplayOnVDD()) {
        if (options.safe) {
            // 切换到物理 GPU 或 Basic Display
            SwitchPrimaryDisplayToPhysical();
        } else {
            // 警告用户
            SetLastError("WARNING: Primary display is on VDD. Use --safe flag to auto-switch.");
        }
    }
    
    // 2. 检查是否有 fallback 路径
    if (!HasFallbackDisplay()) {
        SetLastError("ERROR: No fallback display adapter found. Uninstall may cause black screen.");
        return Status::UnsafeOperation;
    }
    
    // 3. 停止虚拟显示器（如果激活）
    if (IsActive()) {
        Deactivate();
    }
    
    // 4. 等待显示拓扑稳定
    WaitForDisplayTopologyStable(2000); // 2 seconds
    
    // 5. 执行卸载
    return UninstallDriverImpl();
}
```

#### 2.2 显示路径切换

**新增函数**：

```cpp
// 检查主显示器是否在 VDD 上
bool IsPrimaryDisplayOnVDD();

// 切换主显示器到物理 GPU
Status SwitchPrimaryDisplayToPhysical();

// 检查是否有 fallback 显示适配器
bool HasFallbackDisplay();

// 等待显示拓扑稳定
void WaitForDisplayTopologyStable(uint32_t timeoutMs);
```

**使用 Windows Display API**：
- `DisplayConfigGetDeviceInfo()`
- `SetDisplayConfig()` / `ChangeDisplaySettingsEx()`
- `QueryDisplayConfig()`

#### 2.3 ConnectionId/SourceId 冲突检测

**实现位置**：`Driver.cpp` (IDD Sample 驱动)

```cpp
// 生成唯一的 ConnectionId
// 基于：机器 SID + 驱动版本 + 适配器索引 + 会话 ID
UINT64 GenerateUniqueConnectionId(UINT adapterIndex, UINT sessionId) {
    // 哈希算法生成唯一 ID
    // 避免与物理输出 ID 冲突
}
```

#### 2.4 SID 变更防护

**检查点**：
```cpp
// 安装前检查
Status CheckUserContext() {
    // 1. 获取当前用户 SID
    // 2. 检查安装目录的 ACL
    // 3. 如果不一致，警告或提供 --fix-acl
}
```

**命令选项**：
```batch
vddctl install --inf <path> --fix-acl
```

---

### 阶段 3: 事务性回滚（5-7 天）🔄 待实施

**目标**：安装/卸载失败时自动恢复

#### 3.1 状态备份

**安装前备份**：
```cpp
struct DisplayTopologyBackup {
    std::vector<DisplayConfig> configs;
    std::wstring primaryDisplay;
    std::vector<DeviceInfo> existingDevices;
};

Status BackupDisplayTopology(DisplayTopologyBackup& backup);
Status RestoreDisplayTopology(const DisplayTopologyBackup& backup);
```

#### 3.2 回滚逻辑

**安装失败回滚**：
```cpp
Status VddSdkImpl::InstallDriver(const std::wstring& infPath) {
    DisplayTopologyBackup backup;
    
    // 1. 备份当前状态
    if (BackupDisplayTopology(backup) != Status::Ok) {
        SetLastError("Failed to backup display topology");
        return Status::BackupFailed;
    }
    
    // 2. 执行安装
    Status result = InstallDriverImpl(infPath);
    
    // 3. 如果失败，回滚
    if (result != Status::Ok) {
        RestoreDisplayTopology(backup);
        CleanupFailedInstallation();
    }
    
    return result;
}
```

**卸载失败回滚**：
```cpp
Status VddSdkImpl::UninstallDriver() {
    DisplayTopologyBackup backup;
    BackupDisplayTopology(backup);
    
    Status result = UninstallDriverImpl();
    
    if (result != Status::Ok) {
        // 尝试重新安装驱动
        ReinstallDriver();
        RestoreDisplayTopology(backup);
    }
    
    return result;
}
```

---

### 阶段 4: 命令接口增强（2-3 天）🔄 待实施

#### 4.1 命令接口设计

```bash
# 基本命令
vddctl init                                    # 初始化 SDK
vddctl status                                  # 查看状态
vddctl version                                 # 版本信息

# 安装/卸载
vddctl install --inf <path> [--method setupapi] [--silent] [--fix-acl]
vddctl uninstall [--safe] [--rebind]
vddctl repair [--display] [--fallback]

# 高级功能
vddctl stage --inf <path>                      # 仅预置到 DriverStore
vddctl collect-logs --out <path>               # 收集诊断日志

# 显示控制
vddctl activate [--width W] [--height H]      # 激活虚拟显示
vddctl deactivate                              # 停用虚拟显示
vddctl list                                    # 列出所有显示器
vddctl setmode --index N --width W --height H # 改变分辨率
vddctl setlocation --index N --x X --y Y      # 设置位置
vddctl setprimary --index N                   # 设为主显示
```

#### 4.2 选项说明

**安装选项**：
- `--method setupapi|pnputil`: 安装方法（默认 setupapi）
- `--silent`: 静默模式（无 UI）
- `--fix-acl`: 自动修复 ACL 权限

**卸载选项**：
- `--safe`: 安全卸载（自动切换显示路径）
- `--rebind`: 卸载后重新绑定到物理 GPU

**修复选项**：
- `--display`: 修复显示拓扑
- `--fallback`: 使用 fallback 路径恢复

---

### 阶段 5: 日志和诊断（2-3 天）🔄 待实施

#### 5.1 日志系统

**日志位置**：
```
%ProgramData%\VDD SDK\logs\vddctl.log
%ProgramData%\VDD SDK\logs\vddsdk.log
```

**日志内容**：
```
[2025-10-30 18:45:23] [INFO] vddctl init
[2025-10-30 18:45:23] [INFO] SDK Version: 1.0.0
[2025-10-30 18:45:24] [INFO] vddctl install --inf IddSampleDriver_Fixed.inf
[2025-10-30 18:45:24] [INFO] Current User SID: S-1-5-21-...
[2025-10-30 18:45:24] [INFO] Backup display topology: 2 displays
[2025-10-30 18:45:25] [INFO] Creating device: ROOT\IddSampleDriver
[2025-10-30 18:45:26] [INFO] Driver installed successfully
[2025-10-30 18:45:26] [INFO] Device Instance: ROOT\IDDSAMPLEDRIVER\0003
[2025-10-30 18:45:26] [INFO] Device Class: Display {4d36e968-...}
```

#### 5.2 诊断日志收集

**实现 `collect-logs` 命令**：

```cpp
Status CollectDiagnosticLogs(const std::wstring& outputPath) {
    // 1. vddctl.log
    // 2. Windows SetupAPI.dev.log (摘要)
    // 3. dxdiag /t 输出
    // 4. DisplayConfigGetDeviceInfo 输出
    // 5. whoami /user
    // 6. icacls 关键目录
    // 7. Registry ProfileList
    
    // 打包成 ZIP
    CreateDiagnosticZip(outputPath);
}
```

**命令**：
```batch
vddctl collect-logs --out logs-2025-10-30.zip
```

---

### 阶段 6: 安全性和签名（3-5 天）🔄 待实施

#### 6.1 代码签名

**需要签名的文件**：
- `vddctl.exe`
- `vddsdk.dll`
- `IddSampleDriver.dll`
- `IndirectKmd.sys`
- `IddSampleDriver.cat`

**签名流程**：
```batch
REM 使用代码签名证书
signtool sign /f cert.pfx /p password /t http://timestamp.digicert.com vddctl.exe
signtool sign /f cert.pfx /p password /t http://timestamp.digicert.com vddsdk.dll
```

#### 6.2 完整性校验

**在运行时验证**：
```cpp
bool VerifyCodeSignature(const std::wstring& filePath) {
    // 使用 WinVerifyTrust API
    // 验证 Authenticode 签名
}

Status VddSdkImpl::Initialize(const SdkConfig& config) {
    // 验证 DLL 签名
    if (!VerifyCodeSignature(L"vddsdk.dll")) {
        return Status::SecurityError;
    }
    
    // 验证驱动签名
    if (!VerifyCodeSignature(L"IddSampleDriver.dll")) {
        return Status::SecurityError;
    }
    
    // ... 继续初始化
}
```

---

## ⚠️ 风险防护机制

### 1. 黑屏防护

**问题**：卸载 VDD 后主显示器丢失，导致黑屏

**解决方案**：

```cpp
Status SafeUninstall() {
    // 1. 检查主显示器
    if (IsPrimaryDisplayOnVDD()) {
        // 2. 切换到物理 GPU
        SwitchPrimaryDisplayToPhysical();
        
        // 3. 等待切换完成
        WaitForDisplayTopologyStable(2000);
        
        // 4. 验证切换成功
        if (IsPrimaryDisplayOnVDD()) {
            return Status::UnsafeOperation;
        }
    }
    
    // 5. 检查 fallback
    if (!HasFallbackDisplay()) {
        return Status::NoFallbackDisplay;
    }
    
    // 6. 执行卸载
    return UninstallDriverImpl();
}
```

**测试脚本**：
```batch
REM 黑屏回归测试
vddctl install --inf IddSampleDriver_Fixed.inf
vddctl activate --width 1920 --height 1080
vddctl setprimary --index 0    # 设置 VDD 为主显示
vddctl uninstall --safe         # 应该自动切换回物理显示
```

### 2. 鼠标偏移防护

**问题**：ConnectionId/SourceId 冲突导致输入映射错误

**解决方案**：

**在驱动中生成唯一 ID**：
```cpp
// Driver.cpp
UINT64 GenerateUniqueConnectionId(IDDCX_ADAPTER adapter, UINT index) {
    // 方法 1: 使用 GUID
    GUID guid;
    CoCreateGuid(&guid);
    
    // 方法 2: 哈希
    // 机器 SID + 适配器地址 + 索引 + 时间戳
    UINT64 hash = ComputeHash(machineSid, adapterPtr, index, timestamp);
    
    // 确保不与物理输出冲突
    // 物理输出通常使用 0x00000000-0x7FFFFFFF
    // 虚拟输出使用 0x80000000-0xFFFFFFFF
    return 0x8000000000000000ULL | hash;
}
```

### 3. SID 变更防护

**问题**：在不同用户/SYSTEM 下安装导致 ACL 错乱

**解决方案**：

```cpp
Status CheckAndFixACL() {
    // 1. 获取当前用户 SID
    std::wstring currentSid = GetCurrentUserSid();
    
    // 2. 检查安装目录 ACL
    std::wstring installDir = GetInstallDirectory();
    std::wstring ownerSid = GetDirectoryOwner(installDir);
    
    // 3. 如果不一致
    if (currentSid != ownerSid) {
        // 警告
        SetLastError("ACL mismatch detected. Use --fix-acl to repair.");
        
        // 如果指定了 --fix-acl
        if (config.fixAcl) {
            TakeOwnership(installDir, currentSid);
            ResetACL(installDir);
        }
        
        return Status::AclMismatch;
    }
    
    return Status::Ok;
}
```

**命令**：
```batch
vddctl install --inf IddSampleDriver_Fixed.inf --fix-acl
```

---

## 📝 执行流程（详细）

### 安装流程

```
vddctl install --inf <path> --safe --fix-acl
  ↓
[1] 初始化检查
  ├─ 检查管理员权限
  ├─ 检查 INF 文件存在
  ├─ 验证文件签名
  └─ 检查用户 SID 和 ACL
  ↓
[2] 备份当前状态
  ├─ 备份显示拓扑
  ├─ 备份现有 VDD 设备列表
  └─ 创建还原点（可选）
  ↓
[3] 执行安装
  ├─ SetupDiCreateDeviceInfoList
  ├─ SetupDiCreateDeviceInfo
  ├─ SetupDiSetDeviceRegistryProperty (HardwareID)
  ├─ SetupDiCallClassInstaller(DIF_REGISTERDEVICE)
  └─ SetupDiCallClassInstaller(DIF_INSTALLDEVICE)
  ↓
[4] 验证安装
  ├─ 检查设备实例
  ├─ 检查设备类 (Display)
  ├─ 检查设备状态 (OK)
  └─ 等待 PnP 稳定
  ↓
[5] 完成
  ├─ 写入日志
  ├─ 返回状态
  └─ 如果失败 → 回滚
```

### 卸载流程（安全模式）

```
vddctl uninstall --safe --rebind
  ↓
[1] 前置检查
  ├─ 检查管理员权限
  ├─ 检查驱动是否已安装
  ├─ 检查主显示器位置
  └─ 检查 fallback 显示
  ↓
[2] 切换显示路径 (--safe)
  ├─ 如果主显示在 VDD
  │   ├─ 枚举物理 GPU
  │   ├─ 切换主显示到物理 GPU
  │   └─ 等待切换完成
  └─ 验证切换成功
  ↓
[3] 停用虚拟显示
  ├─ Deactivate 所有 VDD 输出
  └─ 等待拓扑更新
  ↓
[4] 备份状态
  └─ 记录当前拓扑（用于回滚）
  ↓
[5] 执行卸载
  ├─ SetupDiGetClassDevs (枚举 Display 类)
  ├─ 查找所有 IddSampleDriver 设备
  ├─ SetupDiCallClassInstaller(DIF_REMOVE)
  └─ 等待设备移除完成
  ↓
[6] 重新绑定 (--rebind)
  ├─ 扫描物理 GPU
  ├─ 触发 PnP 重新枚举
  └─ 确保主显示在物理 GPU
  ↓
[7] 验证
  ├─ 确认 VDD 设备已移除
  ├─ 确认主显示正常
  └─ 确认无黑屏
  ↓
[8] 完成
  ├─ 写入日志
  └─ 如果失败 → 回滚（重新安装 VDD）
```

---

## 🧪 测试剧本

### 测试 1: 基本安装/卸载

```batch
REM 1. 清理环境
vddctl uninstall

REM 2. 安装
vddctl install --inf IddSampleDriver_Fixed.inf

REM 3. 验证
vddctl status
REM 应显示: Driver Installed: Yes

REM 4. 卸载
vddctl uninstall

REM 5. 验证
vddctl status
REM 应显示: Driver Installed: No
```

**预期结果**：✅ 所有步骤成功，无错误

### 测试 2: 黑屏回归测试

```batch
REM 1. 安装并激活
vddctl install --inf IddSampleDriver_Fixed.inf
vddctl activate --width 1920 --height 1080

REM 2. 将 VDD 设为主显示器
vddctl setprimary --index 0

REM 3. 确认主显示在 VDD
vddctl list
REM 应显示: Virtual Display (Primary)

REM 4. 不安全卸载（应该失败或警告）
vddctl uninstall
REM 应显示: WARNING: Primary display is on VDD

REM 5. 安全卸载
vddctl uninstall --safe
REM 应自动切换显示并成功卸载

REM 6. 验证
REM 系统应该没有黑屏，主显示在物理 GPU
```

**预期结果**：✅ 无黑屏，显示正常切换

### 测试 3: 安装失败回滚

```batch
REM 1. 备份当前状态
vddctl status > before.txt

REM 2. 尝试安装损坏的 INF（模拟失败）
vddctl install --inf broken.inf

REM 3. 验证回滚
vddctl status > after.txt
fc before.txt after.txt
REM 应该相同，说明回滚成功
```

**预期结果**：✅ 安装失败后自动回滚，系统状态不变

### 测试 4: SID/ACL 问题

```batch
REM 1. 以 Admin 身份安装
runas /user:Administrator "vddctl install --inf IddSampleDriver_Fixed.inf"

REM 2. 以普通用户检查
vddctl status
REM 应该检测到 ACL 问题并警告

REM 3. 修复 ACL
vddctl install --inf IddSampleDriver_Fixed.inf --fix-acl

REM 4. 验证
vddctl status
REM 应该正常
```

**预期结果**：✅ 检测并修复 ACL 问题

---

## 📦 交付物

### 阶段 1（当前）
- [x] `vddsdk.cpp` - InstallDriver/UninstallDriver 实现
- [x] `vddctl.cpp` - 命令行工具框架
- [ ] `vddctl.exe` - 编译的可执行文件
- [ ] 基本安装/卸载测试报告

### 阶段 2
- [ ] 安全卸载实现（`--safe`）
- [ ] 显示路径切换功能
- [ ] ConnectionId 冲突检测
- [ ] SID/ACL 检查和修复
- [ ] 安全功能测试报告

### 阶段 3
- [ ] 事务性回滚实现
- [ ] 状态备份/恢复
- [ ] 失败自动恢复
- [ ] 回滚测试报告

### 阶段 4
- [ ] 完整命令接口
- [ ] 所有选项支持
- [ ] 命令帮助和文档
- [ ] 用户手册

### 阶段 5
- [ ] 日志系统
- [ ] 诊断日志收集
- [ ] `collect-logs` 命令
- [ ] 故障诊断指南

### 阶段 6
- [ ] 代码签名
- [ ] 完整性校验
- [ ] 安全测试报告
- [ ] 发布版本

---

## 🎯 当前优先级

### P0 - 立即完成
1. ✅ 修复 `vddctl.cpp` 参数 bug
2. 🔄 编译 `vddctl.exe`
3. 🔄 测试基本安装/卸载

### P1 - 本周完成
4. 实现 `--safe` 卸载
5. 添加显示路径切换
6. 测试黑屏防护

### P2 - 下周完成
7. 实现事务性回滚
8. 添加日志系统
9. 完整测试套件

---

## 📞 下一步行动

**我可以立即帮你做的**：

1. **修复编译问题** - 创建正确的编译脚本，成功编译 `vddctl.exe`

2. **添加安全卸载** - 在 `vddsdk.cpp` 中实现 `--safe` 逻辑

3. **创建测试脚本** - 黑屏回归测试等

4. **更新文档** - 将这个路线图整合到项目文档

**你想先做哪一个？**



