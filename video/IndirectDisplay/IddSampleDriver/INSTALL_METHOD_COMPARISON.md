# SetupAPI 安装方法对比分析

## 🎯 测试结果

| 方法 | 文件 | 结果 | Device Manager 可见 | Class 设置 |
|------|------|------|-------------------|-----------|
| **简单方法** | `setupapi_install_simple.cpp` | ✅ 成功 | ✅ 是 | ✅ Display |
| **回滚方法** | `setupapi_install_with_rollback.cpp` | ⚠️ 部分 | ❌ 否 | ❌ 空 |

---

## 🔍 代码对比

### 共同点

两个版本都执行：

```cpp
// 步骤 1-4：完全相同
1. SetupDiCreateDeviceInfoList(&displayClassGuid, NULL)
2. SetupDiCreateDeviceInfoW(...)
3. SetupDiSetDeviceRegistryPropertyW(SPDRP_HARDWAREID, ...)
4. SetupDiCallClassInstaller(DIF_REGISTERDEVICE, ...)
```

### 关键差异：步骤 5（安装驱动）

#### ✅ 简单方法（成功）

```cpp
// 步骤 5: 直接安装
if (!SetupDiCallClassInstaller(DIF_INSTALLDEVICE, deviceInfoSet, &devInfoData)) {
    // DIF_INSTALLDEVICE 失败 ❌
    
    // 备用方法
    UpdateDriverForPlugAndPlayDevicesW(
        NULL,
        L"ROOT\\IddSampleDriver",
        fullPath,                    // ✅ 完整路径
        INSTALLFLAG_FORCE,
        &reboot
    );
    // ✅ 这个成功了，并且设置了 Class!
}
```

**特点**：
- ❌ `DIF_INSTALLDEVICE` 失败（但不影响）
- ✅ `UpdateDriverForPlugAndPlayDevices` 成功
- ✅ **自动从 INF 读取 Class 信息**
- ✅ Device Manager 可见

---

#### ❌ 回滚方法（失败）

```cpp
// 步骤 5: 复杂安装流程
// [1] 设置 INF 路径
SP_DEVINSTALL_PARAMS_W deviceInstallParams = {};
deviceInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS_W);
SetupDiGetDeviceInstallParamsW(deviceInfoSet, &devInfoData, &deviceInstallParams);
wcsncpy_s(deviceInstallParams.DriverPath, MAX_PATH, infPath, _TRUNCATE);
deviceInstallParams.Flags |= DI_ENUMSINGLEINF;
SetupDiSetDeviceInstallParamsW(deviceInfoSet, &devInfoData, &deviceInstallParams);

// [2] 构建驱动列表
SetupDiBuildDriverInfoList(deviceInfoSet, &devInfoData, SPDIT_CLASSDRIVER);

// [3] 选择驱动
SetupDiCallClassInstaller(DIF_SELECTBESTCOMPATDRV, deviceInfoSet, &devInfoData);

// [4] 安装
if (!SetupDiCallClassInstaller(DIF_INSTALLDEVICE, deviceInfoSet, &devInfoData)) {
    // DIF_INSTALLDEVICE 仍然失败 ❌
    
    // 备用方法
    UpdateDriverForPlugAndPlayDevicesW(
        NULL,
        L"ROOT\\IddSampleDriver",
        infPath,                     // ✅ 完整路径
        INSTALLFLAG_FORCE,
        &reboot
    );
    // ✅ 这个成功了，但 Class 没有设置！❌
}
```

**特点**：
- ❌ `DIF_INSTALLDEVICE` 仍然失败
- ✅ `UpdateDriverForPlugAndPlayDevices` 成功创建设备
- ❌ **但 Class 没有设置**
- ❌ Device Manager 看不到

---

## 🤔 为什么会有差异？

### 理论分析

**简单方法成功的原因**：

1. **没有预先设置参数**
   - SetupAPI 使用默认行为
   - Windows 自动搜索驱动存储区
   - `UpdateDriverForPlugAndPlayDevices` 能正确解析 INF

2. **UpdateDriverForPlugAndPlayDevices 的"干净"调用**
   ```cpp
   // 设备节点已存在（步骤 1-4 创建）
   // 调用此函数时，Windows 会：
   // - 扫描 INF 文件
   // - 读取 [Version] 的 Class 和 ClassGuid
   // - 自动设置到设备节点
   ```

---

**回滚方法失败的原因**：

1. **预先设置了参数干扰了后续流程**
   ```cpp
   deviceInstallParams.Flags |= DI_ENUMSINGLEINF;  // ⚠️ 问题可能在这
   SetupDiBuildDriverInfoList(...)                   // ⚠️ 或者这里
   SetupDiCallClassInstaller(DIF_SELECTBESTCOMPATDRV, ...) // ⚠️ 或者这里
   ```

2. **可能的问题**：
   - 设置了 `DI_ENUMSINGLEINF` 后，驱动列表构建失败
   - `SPDIT_CLASSDRIVER` 参数可能不正确（应该用 `SPDIT_COMPATDRIVER`？）
   - `DIF_SELECTBESTCOMPATDRV` 选择了错误的驱动或没有选择任何驱动
   - 这些步骤**污染了设备节点的状态**

3. **后续的 UpdateDriverForPlugAndPlayDevices**：
   ```cpp
   // 此时设备节点的状态已经被修改了
   // UpdateDriverForPlugAndPlayDevices 可能：
   // - 看到设备已经"部分安装"
   // - 跳过了 Class 设置步骤
   // - 只更新了驱动文件，没有更新元数据
   ```

---

## 🔬 验证假设的方法

### 测试 1：移除参数设置

```cpp
// 在回滚方法中，注释掉这些行：
// SP_DEVINSTALL_PARAMS_W deviceInstallParams = {};
// ...（所有参数设置代码）

// 直接调用：
if (!SetupDiCallClassInstaller(DIF_INSTALLDEVICE, deviceInfoSet, &devInfoData)) {
    UpdateDriverForPlugAndPlayDevicesW(...);  // 应该会成功
}
```

**预期结果**：Class 应该会被正确设置

---

### 测试 2：使用 SPDIT_COMPATDRIVER

```cpp
// 改为：
SetupDiBuildDriverInfoList(deviceInfoSet, &devInfoData, SPDIT_COMPATDRIVER);
//                                                       ^^^^^^^^^^^^^^^^^^
// 而不是 SPDIT_CLASSDRIVER
```

**预期结果**：可能会找到正确的驱动

---

### 测试 3：检查驱动列表

```cpp
// 在构建驱动列表后，枚举查看：
SP_DRVINFO_DATA_W drvInfoData = {};
drvInfoData.cbSize = sizeof(SP_DRVINFO_DATA_W);

for (DWORD i = 0; SetupDiEnumDriverInfoW(deviceInfoSet, &devInfoData, 
        SPDIT_CLASSDRIVER, i, &drvInfoData); i++) {
    wprintf(L"Driver %d: %s\n", i, drvInfoData.Description);
}
```

**预期结果**：查看是否找到了驱动

---

## 💡 推荐的修复方案

### 方案 A：使用简单方法（推荐） ✅

```cpp
// 保持简单，不要过度配置
// 直接使用 setupapi_install_simple.cpp 的逻辑
```

**优点**：
- ✅ 已验证可用
- ✅ 代码简洁
- ✅ 易于维护

**缺点**：
- ❌ 没有详细的回滚逻辑
- ❌ 错误处理较简单

---

### 方案 B：修复回滚方法

#### 选项 1：完全移除参数设置

```cpp
// 在 setupapi_install_with_rollback.cpp 中
// 步骤 5: 安装驱动程式
wprintf(L"[5/5] 安裝驅動程式...\n");

// ❌ 移除这些：
// SP_DEVINSTALL_PARAMS_W deviceInstallParams = {};
// SetupDiGetDeviceInstallParamsW(...);
// SetupDiBuildDriverInfoList(...);
// SetupDiCallClassInstaller(DIF_SELECTBESTCOMPATDRV, ...);

// ✅ 直接调用：
if (!SetupDiCallClassInstaller(DIF_INSTALLDEVICE, deviceInfoSet, &devInfoData)) {
    // 备用方法
    UpdateDriverForPlugAndPlayDevicesW(...);
}
```

#### 选项 2：只在 UpdateDriverForPlugAndPlayDevices 之前清理状态

```cpp
// 如果 DIF_INSTALLDEVICE 失败，在调用 UpdateDriverForPlugAndPlayDevices 之前：
// 清理可能被污染的参数
SP_DEVINSTALL_PARAMS_W cleanParams = {};
cleanParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS_W);
SetupDiSetDeviceInstallParamsW(deviceInfoSet, &devInfoData, &cleanParams);

// 销毁驱动列表
SetupDiDestroyDriverInfoList(deviceInfoSet, &devInfoData, SPDIT_CLASSDRIVER);

// 然后调用
UpdateDriverForPlugAndPlayDevicesW(...);
```

---

## 📋 总结

### 简单方法为什么有效

```
1. 创建设备节点（无 Class）
   └─> SetupDiCreateDeviceInfo
   └─> SetupDiSetDeviceRegistryProperty(HARDWAREID)
   └─> SetupDiCallClassInstaller(DIF_REGISTERDEVICE)

2. 尝试安装驱动（失败，但不影响）
   └─> SetupDiCallClassInstaller(DIF_INSTALLDEVICE) ❌

3. 备用方法（成功！）
   └─> UpdateDriverForPlugAndPlayDevicesW
       └─> Windows 自动：
           ├─> 读取 INF 的 [Version] 段
           ├─> 设置 Class = Display
           ├─> 设置 ClassGuid = {4d36e968...}
           └─> 安装驱动文件
```

### 回滚方法为什么失败

```
1. 创建设备节点（相同）
   └─> 步骤 1-4 同简单方法

2. 设置复杂参数（污染状态）
   └─> SetupDiSetDeviceInstallParams(DI_ENUMSINGLEINF)
   └─> SetupDiBuildDriverInfoList(SPDIT_CLASSDRIVER) ⚠️
   └─> SetupDiCallClassInstaller(DIF_SELECTBESTCOMPATDRV) ⚠️

3. 尝试安装驱动（失败）
   └─> SetupDiCallClassInstaller(DIF_INSTALLDEVICE) ❌

4. 备用方法（部分成功）
   └─> UpdateDriverForPlugAndPlayDevicesW
       └─> Windows 看到设备"已部分配置"
       └─> 只更新驱动文件
       └─> ❌ 跳过了 Class 设置
```

---

## 🎯 建议

**立即采用**：
1. 使用 `setupapi_install_simple.cpp` 作为基础
2. 添加回滚逻辑（只在失败时移除设备节点）
3. 不要尝试"帮助" SetupAPI 选择驱动

**代码示例**：
```cpp
// 正确的回滚版本应该是：
bool deviceRegistered = false;

// 步骤 1-4：创建和注册设备
// ...
deviceRegistered = true;

// 步骤 5：简单安装（不设置额外参数）
if (!SetupDiCallClassInstaller(DIF_INSTALLDEVICE, deviceInfoSet, &devInfoData)) {
    // 备用方法
    if (!UpdateDriverForPlugAndPlayDevicesW(...)) {
        // ❌ 两种方法都失败 - 执行回滚
        if (deviceRegistered) {
            SetupDiCallClassInstaller(DIF_REMOVE, deviceInfoSet, &devInfoData);
        }
        return false;
    }
}

// ✅ 成功
return true;
```

---

## 🔑 关键结论

**不要过度配置 SetupAPI！**

- ✅ Windows 知道如何安装驱动
- ✅ INF 文件已经包含所有必要信息
- ❌ 手动设置参数可能干扰自动流程
- ❌ 预先选择驱动可能选择错误的驱动

**简单就是好！**



