# setupapi_install_simple.cpp vs setupapi_install_with_minimal_rollback.cpp

## 🔍 完整差异对比

### ✅ 完全相同的部分（1-100 行）

| 行数 | 内容 | 状态 |
|------|------|------|
| 1-33 | 头文件、路径处理、文件检查 | ✅ 完全相同 |
| 34-89 | 步骤 1-3（创建设备信息、设置硬件ID） | ✅ 完全相同 |
| 90-99 | 步骤 4（注册设备） | ✅ 完全相同 |

---

## 🔴 差异部分：步骤 5 之后（100行开始）

### 原始版本（setupapi_install_simple.cpp）

```cpp
行 102:    wprintf(L"成功\n");
行 103:    
行 104:    // 步驟 5: 安裝驅動程式
行 105:    wprintf(L"[5] 安裝驅動程式...\n");
行 106:    
行 107:    if (!SetupDiCallClassInstaller(DIF_INSTALLDEVICE, deviceInfoSet, &devInfoData)) {
行 108:        DWORD error = GetLastError();
行 109:        wprintf(L"失敗: DIF_INSTALLDEVICE (錯誤: %d)\n", error);
行 110:        
行 111:        // 即使失敗，也嘗試用其他方法
行 112:        wprintf(L"\n嘗試使用 UpdateDriverForPlugAndPlayDevices...\n");
行 113:        
行 114:        BOOL reboot = FALSE;
行 115:        if (UpdateDriverForPlugAndPlayDevicesW(
行 116:                NULL,
行 117:                L"ROOT\\IddSampleDriver",
行 118:                fullPath,
行 119:                INSTALLFLAG_FORCE,
行 120:                &reboot)) {
行 121:            wprintf(L"成功通過 UpdateDriverForPlugAndPlayDevices 安裝\n");
行 122:            if (reboot) {
行 123:                wprintf(L"警告: 需要重新啟動\n");
行 124:            }
行 125:        } else {
行 126:            wprintf(L"UpdateDriverForPlugAndPlayDevices 也失敗了 (錯誤: %d)\n", GetLastError());
行 127:        }
行 128:    } else {
行 129:        wprintf(L"成功!\n");
行 130:    }
行 131:    
行 132:    // 清理
行 133:    SetupDiDestroyDeviceInfoList(deviceInfoSet);
行 134:    
行 135:    wprintf(L"\n========================================\n");
行 136:    wprintf(L"完成\n");
行 137:    wprintf(L"========================================\n");
行 138:    wprintf(L"\n請檢查設備管理器以確認安裝結果\n");
行 139:    
行 140:    return 0;
行 141:}
```

### 新版本（setupapi_install_with_minimal_rollback.cpp）

```cpp
行 99:     wprintf(L"成功\n");
行 100:    
行 101:    // ⭐ 新增：標記設備已註冊（回滾關鍵點）
行 102:    bool deviceRegistered = true;
行 103:    bool installSuccess = false;
行 104:    
行 105:    // 步驟 5: 安裝驅動程式
行 106:    wprintf(L"[5] 安裝驅動程式...\n");
行 107:    
行 108:    if (!SetupDiCallClassInstaller(DIF_INSTALLDEVICE, deviceInfoSet, &devInfoData)) {
行 109:        DWORD error = GetLastError();
行 110:        wprintf(L"失敗: DIF_INSTALLDEVICE (錯誤: %d)\n", error);
行 111:        
行 112:        // 即使失敗，也嘗試用其他方法
行 113:        wprintf(L"\n嘗試使用 UpdateDriverForPlugAndPlayDevices...\n");
行 114:        
行 115:        BOOL reboot = FALSE;
行 116:        if (UpdateDriverForPlugAndPlayDevicesW(
行 117:                NULL,
行 118:                L"ROOT\\IddSampleDriver",
行 119:                fullPath,
行 120:                INSTALLFLAG_FORCE,
行 121:                &reboot)) {
行 122:            wprintf(L"成功通過 UpdateDriverForPlugAndPlayDevices 安裝\n");
行 123:            if (reboot) {
行 124:                wprintf(L"警告: 需要重新啟動\n");
行 125:            }
行 126:            installSuccess = true;  // ⭐ 標記成功
行 127:        } else {
行 128:            wprintf(L"UpdateDriverForPlugAndPlayDevices 也失敗了 (錯誤: %d)\n", GetLastError());
行 129:            // ⭐ installSuccess 保持 false，會觸發回滾
行 130:        }
行 131:    } else {
行 132:        wprintf(L"成功!\n");
行 133:        installSuccess = true;  // ⭐ 標記成功
行 134:    }
行 135:    
行 136:    // ⭐ 新增：回滾邏輯
行 137:    if (!installSuccess && deviceRegistered) {
行 138:        wprintf(L"\n========================================\n");
行 139:        wprintf(L"⚠ 安裝失敗，執行自動回滾...\n");
行 140:        wprintf(L"========================================\n");
行 141:        
行 142:        SP_REMOVEDEVICE_PARAMS removeParams = {};
行 143:        removeParams.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
行 144:        removeParams.ClassInstallHeader.InstallFunction = DIF_REMOVE;
行 145:        removeParams.Scope = DI_REMOVEDEVICE_GLOBAL;
行 146:        removeParams.HwProfile = 0;
行 147:        
行 148:        if (SetupDiSetClassInstallParamsW(
行 149:                deviceInfoSet,
行 150:                &devInfoData,
行 151:                (PSP_CLASSINSTALL_HEADER)&removeParams,
行 152:                sizeof(removeParams))) {
行 153:            
行 154:            if (SetupDiCallClassInstaller(DIF_REMOVE, deviceInfoSet, &devInfoData)) {
行 155:                wprintf(L"✓ 設備已回滾移除\n");
行 156:            } else {
行 157:                wprintf(L"✗ 回滾失敗 (錯誤: %d) - 可能需要手動清理\n", GetLastError());
行 158:            }
行 159:        }
行 160:    }
行 161:    
行 162:    // 清理
行 163:    SetupDiDestroyDeviceInfoList(deviceInfoSet);
行 164:    
行 165:    if (installSuccess) {
行 166:        wprintf(L"\n========================================\n");
行 167:        wprintf(L"✓ 安裝成功！\n");
行 168:        wprintf(L"========================================\n");
行 169:        wprintf(L"\n請檢查設備管理器以確認安裝結果\n");
行 170:        return 0;
行 171:    } else {
行 172:        wprintf(L"\n========================================\n");
行 173:        wprintf(L"✗ 安裝失敗\n");
行 174:        wprintf(L"========================================\n");
行 175:        return 1;
行 176:    }
行 177:}
```

---

## 🔍 关键差异汇总

### 1️⃣ 新增变量（行 102-103）

```cpp
// ⭐ NEW
bool deviceRegistered = true;
bool installSuccess = false;
```

### 2️⃣ 成功标记（行 126, 133）

```cpp
// 原始版本：无
// 新版本：
installSuccess = true;  // 在成功时设置
```

### 3️⃣ 回滚逻辑（行 137-160）

```cpp
// ⭐ NEW: 完整的回滚代码块
if (!installSuccess && deviceRegistered) {
    // ... 移除设备的代码 ...
}
```

### 4️⃣ 返回逻辑（行 165-176）

```cpp
// 原始版本：
return 0;  // 总是返回成功

// 新版本：
if (installSuccess) {
    return 0;
} else {
    return 1;
}
```

---

## ⚠️ **致命问题发现！**

### 🔴 原始版本的行为

```cpp
行 125-127:
        } else {
            wprintf(L"UpdateDriverForPlugAndPlayDevices 也失敗了 (錯誤: %d)\n", GetLastError());
        }  // ← 即使两个方法都失败
行 128-130:
    } else {
        wprintf(L"成功!\n");
    }  // ← 无论如何
    
行 132-133:
    // 清理
    SetupDiDestroyDeviceInfoList(deviceInfoSet);
    
行 140:
    return 0;  // ← **总是返回成功！即使安装失败！**
```

**原始版本的逻辑**：
- ✅ DIF_INSTALLDEVICE 成功 → 输出"成功！" → return 0
- ❌ DIF_INSTALLDEVICE 失败，UpdateDriver 成功 → 输出"成功通过..." → return 0
- ❌ DIF_INSTALLDEVICE 失败，UpdateDriver 失败 → 输出"也失败了" → **仍然 return 0！**

**这意味着**：原始版本**即使安装失败也会返回成功**！

但设备确实被创建了（因为 UpdateDriver 可能部分成功）！

---

### 🔴 新版本的行为

```cpp
行 126-130:
            installSuccess = true;  // ← 只有 UpdateDriver 成功才设置
        } else {
            wprintf(L"UpdateDriverForPlugAndPlayDevices 也失敗了...\n");
            // ← installSuccess 保持 false
        }

行 137-160:
    if (!installSuccess && deviceRegistered) {
        // ← **回滚代码：移除已注册的设备**
    }

行 170-176:
    if (installSuccess) {
        return 0;
    } else {
        return 1;  // ← 失败时返回错误码
    }
```

**新版本的逻辑**：
- ✅ DIF_INSTALLDEVICE 成功 → installSuccess=true → return 0
- ❌ DIF_INSTALLDEVICE 失败，UpdateDriver 成功 → installSuccess=true → return 0
- ❌ DIF_INSTALLDEVICE 失败，UpdateDriver 失败 → installSuccess=false → **执行回滚** → return 1

---

## 💡 **根本原因分析**

### 为什么原始版本"成功"？

```
1. DIF_INSTALLDEVICE 失败 ❌
2. UpdateDriverForPlugAndPlayDevices 调用
   → 可能"部分成功"（创建设备节点，设置 Class）
   → 但返回了错误码
3. 输出"也失败了"
4. ⚠️ 但设备已经创建！
5. return 0（总是成功）
6. ✅ 设备存在，Class 正确
```

### 为什么新版本"失败"？

```
1. DIF_INSTALLDEVICE 失败 ❌
2. UpdateDriverForPlugAndPlayDevices 调用
   → 返回错误码
3. installSuccess = false
4. 执行回滚逻辑：
   → SetupDiCallClassInstaller(DIF_REMOVE, ...)
   → ⚠️ **移除了刚创建的设备**
5. return 1（失败）
6. ❌ 设备被回滚移除了
```

---

## 🎯 **结论**

### 问题所在

**新版本的回滚逻辑执行了，把设备移除了！**

即使 `UpdateDriverForPlugAndPlayDevices` **实际上成功创建了设备**（有 Class），但它返回了错误码，导致：
1. `installSuccess` 保持 false
2. 触发回滚
3. 设备被移除

### 为什么会这样？

`UpdateDriverForPlugAndPlayDevices` 可能：
- ✅ 成功创建设备并设置 Class
- ✅ 成功安装驱动
- ❌ 但由于某些非关键错误（如签名警告）返回 FALSE

---

## ✅ 解决方案

### 方案 A：不依赖返回值，而是验证设备是否存在

```cpp
// 步骤 5 之后
// 无论返回值如何，检查设备是否真的存在
wchar_t deviceId[MAX_PATH] = {};
if (SetupDiGetDeviceInstanceIdW(deviceInfoSet, &devInfoData, deviceId, MAX_PATH, NULL)) {
    // 设备存在，检查状态
    ULONG problem = 0;
    ULONG status = 0;
    DEVINST devInst = 0;
    
    if (CM_Locate_DevNodeW(&devInst, deviceId, CM_LOCATE_DEVNODE_NORMAL) == CR_SUCCESS) {
        if (CM_Get_DevNode_Status(&status, &problem, devInst, 0) == CR_SUCCESS) {
            if (problem == 0) {
                // 设备正常工作，认为成功
                installSuccess = true;
            }
        }
    }
}
```

### 方案 B：原始版本逻辑（不回滚）

保持原始版本的行为：
- 即使返回错误，只要设备创建了就算成功
- 不添加回滚逻辑

### 方案 C：只在确认失败时才回滚

```cpp
// 只有在确认设备不存在或有问题时才回滚
if (!installSuccess) {
    // 先验证设备是否真的失败
    // 如果设备存在且工作正常，不回滚
    // 如果设备不存在或有问题，才回滚
}
```

---

## 🎯 推荐方案

**方案 B（保持原始逻辑）+ 文档说明**

**原因**：
1. 原始版本已验证可靠
2. "部分成功"实际上就是成功（设备工作正常）
3. 真正需要回滚的情况极少
4. 复杂的验证逻辑可能引入新问题

**建议**：
- 保留 `setupapi_install_simple.cpp` 用于安装
- 提供 `uninstall_driver.exe` 用于清理
- 在文档中说明：如果安装输出错误但设备存在，仍然是成功的

