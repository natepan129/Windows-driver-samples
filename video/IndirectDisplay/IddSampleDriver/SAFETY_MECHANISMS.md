# VDD SDK 安全機制文檔

## 概述

VDD SDK 在安裝/卸載驅動程式時實施了多層安全保護機制，確保操作失敗時系統能夠自動恢復到穩定狀態。

---

## 🛡️ InstallDriver() 安全機制

### 自動回滾 (Auto-Rollback)

**實施位置**: `vddsdk.cpp::VddSdkImpl::InstallDriver()`

**關鍵點**: 設備註冊後（Step 4 之後），任何失敗都會觸發自動回滾

### 工作流程

```
Step 1: Create device info list
  ↓ [失敗] → 返回錯誤，無副作用
Step 2: Create device info
  ↓ [失敗] → 清理資源，返回錯誤
Step 3: Set hardware ID
  ↓ [失敗] → 清理資源，返回錯誤
Step 4: Register device ←━━━ CRITICAL POINT
  ↓ [失敗] → 清理資源，返回錯誤
  ✓ [成功] → deviceRegistered = true
Step 5: Install driver
  ↓ [失敗] → 嘗試 fallback 方法
  ↓ [fallback 失敗] → goto rollback
  
rollback:
  自動移除已註冊的設備
  清理註冊表殘留
  返回錯誤狀態

cleanup:
  釋放所有資源句柄
```

### 安全特性

| 特性 | 描述 | 狀態 |
|------|------|------|
| **狀態追蹤** | 使用 `deviceRegistered` 標誌追蹤關鍵點 | ✅ |
| **自動回滾** | 設備註冊後失敗自動移除設備 | ✅ |
| **資源清理** | 使用 goto cleanup 模式確保資源釋放 | ✅ |
| **錯誤報告** | 詳細錯誤信息包含回滾狀態 | ✅ |
| **零殭屍設備** | 防止留下未安裝驅動的設備節點 | ✅ |

### 代碼示例

```cpp
// 關鍵安全代碼
bool deviceRegistered = false;

// Step 4: 註冊設備（臨界點）
if (!SetupDiCallClassInstaller(DIF_REGISTERDEVICE, deviceInfoSet, &devInfoData)) {
    goto cleanup;  // 還沒註冊，直接清理
}
deviceRegistered = true;  // 標記已註冊

// Step 5: 安裝驅動
if (!SetupDiCallClassInstaller(DIF_INSTALLDEVICE, deviceInfoSet, &devInfoData)) {
    // 嘗試 fallback
    if (!UpdateDriverForPlugAndPlayDevices(...)) {
        goto rollback;  // 兩種方法都失敗，需要回滾
    }
}

rollback:
    if (deviceRegistered) {
        // 自動移除已註冊的設備
        SP_REMOVEDEVICE_PARAMS removeParams = {...};
        SetupDiCallClassInstaller(DIF_REMOVE, deviceInfoSet, &devInfoData);
    }
```

---

## 🗑️ UninstallDriver() 安全機制

### 完整清理 (Complete Cleanup)

**實施位置**: `vddsdk.cpp::VddSdkImpl::UninstallDriver()`

**關鍵特性**: 
1. 找到**所有**相關設備（不遺漏）
2. 批量移除（避免迭代器失效）
3. 部分成功處理

### 工作流程

```
Step 1: 枚舉所有顯示設備
  ↓
Step 2: 收集匹配設備
  - 檢查 InstanceId 包含 "IddSampleDriver"
  - 檢查 FriendlyName 包含 "IddSampleDriver"
  - 大小寫不敏感匹配
  ↓
Step 3: 批量移除
  for each device:
    嘗試移除
    記錄成功/失敗計數
  ↓
Step 4: 報告結果
  - 成功數量
  - 失敗數量
  - 部分成功也返回 Ok
```

### 安全特性

| 特性 | 描述 | 狀態 |
|------|------|------|
| **多重檢查** | InstanceId + FriendlyName 雙重檢查 | ✅ |
| **批量收集** | 先收集再刪除，避免迭代器問題 | ✅ |
| **完整清理** | 移除所有匹配設備，不遺漏 | ✅ |
| **部分成功** | 即使部分失敗也報告成功項 | ✅ |
| **詳細報告** | 返回成功/失敗計數和錯誤詳情 | ✅ |

### 改進點

**Before (舊版本)**:
```cpp
// ❌ 只找第一個設備
while (SetupDiEnumDeviceInfo(...)) {
    if (deviceIdStr.find(L"IddSampleDriver") != ...) {
        found = TRUE;
        break;  // 只移除第一個！
    }
}
```

**After (新版本)**:
```cpp
// ✅ 找到所有設備
std::vector<SP_DEVINFO_DATA> devicesToRemove;
while (SetupDiEnumDeviceInfo(...)) {
    // 檢查 InstanceId 和 FriendlyName
    if (matches) {
        devicesToRemove.push_back(devInfoData);  // 收集全部
    }
}

// 批量移除
for (auto& devInfo : devicesToRemove) {
    // 移除每一個
}
```

---

## 🔧 獨立工具的額外安全層

### install_driver.exe (setupapi_install_with_rollback.cpp)

**額外功能**:
- ✅ **狀態備份**: 安裝前保存系統狀態到 `driver_install_backup.txt`
- ✅ **用戶確認回滾**: 失敗時詢問用戶是否回滾
- ✅ **驗證安裝**: 安裝後驗證設備是否正常工作
- ✅ **詳細日誌**: 每步操作的詳細輸出

### uninstall_driver.exe (setupapi_uninstall.cpp)

**額外功能**:
- ✅ **多步驟清理**:
  1. 移除設備節點
  2. 清理驅動程式存儲區（pnputil）
  3. 清理註冊表殘留
  4. 驗證卸載完成
- ✅ **完整報告**: 每步操作的狀態
- ✅ **強制清理**: 即使部分步驟失敗也繼續

---

## 📊 安全機制對比表

| 功能 | vddsdk.dll API | install_driver.exe | uninstall_driver.exe |
|------|----------------|--------------------|--------------------|
| **自動回滾** | ✅ 永遠啟用 | ✅ 永遠啟用 | N/A |
| **狀態備份** | ❌ 不支持 | ✅ 永遠啟用 | ❌ 不支持 |
| **用戶確認** | ❌ 不支持 | ✅ 互動式 | ❌ 不支持 |
| **資源清理** | ✅ | ✅ | ✅ |
| **完整掃描** | ✅ | ✅ | ✅ |
| **多步驟清理** | ❌ | ❌ | ✅ |
| **詳細日誌** | ✅ | ✅ | ✅ |
| **部分成功處理** | ✅ | ❌ | ✅ |

---

## 🎯 使用建議

### 何時使用 vddsdk.dll API

```cpp
// 程式化控制，自動回滾保證零殘留
vdd::Status status = vdd::InstallDriver(L"driver.inf");
if (status != vdd::Status::Ok) {
    // 自動回滾已執行，設備已清理
    std::cout << "Install failed: " << vdd::GetLastError() << std::endl;
}

// 啟動時自動修復殘留狀態
vdd::RecoverOrphanedState();
```

**適用場景**:
- ✅ 自動化部署（生產環境）
- ✅ 靜默安裝（終端用戶應用）
- ✅ 程式化控制（集成到其他應用）
- ✅ 高性能需求（無額外 I/O）

### 何時使用獨立工具

```batch
REM 互動式安裝，帶狀態備份和用戶確認
install_driver.exe IddSampleDriver.inf

REM 完整卸載，包括驅動存儲區清理
uninstall_driver.exe
```

**適用場景**:
- ✅ 開發/測試（需要查看詳細過程）
- ✅ IT 管理（需要狀態備份和審計）
- ✅ 故障排除（需要用戶確認和詳細日誌）
- ✅ 合規審計（需要變更記錄）

---

## 🐛 故障恢復指南

### 如果 InstallDriver() 失敗

1. **自動回滾已執行**: 系統應該已經恢復，無殘留設備
2. **檢查錯誤信息**: 調用 `GetLastError()` 查看詳情
   ```cpp
   if (vdd::InstallDriver(infPath) != vdd::Status::Ok) {
       std::cout << vdd::GetLastError() << std::endl;
   }
   ```
3. **手動驗證**（可選）: 
   ```powershell
   Get-PnpDevice | Where-Object { $_.FriendlyName -like "*IddSampleDriver*" }
   ```
4. **如有殘留**（極罕見）: 
   - 運行 `vdd::RecoverOrphanedState()` 自動清理
   - 或使用 `uninstall_driver.exe` 強制清理

### 如果 UninstallDriver() 部分失敗

1. **檢查返回狀態**: 即使部分失敗，成功移除的設備已經清理
2. **重新運行**: 可以安全地多次運行（冪等性）
3. **使用獨立工具**: `uninstall_driver.exe` 有更完整的清理流程

### 如果應用崩潰後有殘留

1. **啟動時自動修復**:
   ```cpp
   // 應用啟動時調用
   vdd::RecoverOrphanedState();  // 自動清理並恢復拓撲
   ```
2. **會話管理保護**:
   ```cpp
   // 使用 Heartbeat 防止崩潰殘留
   vdd::BeginSession(lease);
   // ... broker 會在 heartbeat 停止時自動清理 ...
   ```

---

## ✅ 測試覆蓋

### InstallDriver 測試場景

- ✅ INF 文件不存在 → 返回 InvalidArg
- ✅ 設備創建失敗 → 清理並返回錯誤
- ✅ 設備註冊成功但驅動安裝失敗 → 自動回滾移除設備
- ✅ 完整安裝成功 → 設備正常工作

### UninstallDriver 測試場景

- ✅ 沒有設備 → 返回 NotFound
- ✅ 單個設備 → 成功移除
- ✅ 多個設備 → 全部移除
- ✅ 部分設備移除失敗 → 報告部分成功

---

## 📝 變更日誌

### v1.1.0 - 2025-10-30

- ✅ **InstallDriver**: 添加自動回滾機制
- ✅ **UninstallDriver**: 改進為批量移除所有設備
- ✅ **UninstallDriver**: 添加 InstanceId + FriendlyName 雙重檢查
- ✅ **UninstallDriver**: 支持部分成功報告

### v1.0.0 - 之前

- ✅ 基本安裝/卸載功能
- ✅ 資源清理
- ❌ 無自動回滾
- ❌ 可能遺漏設備

---

## 🔒 安全保證

1. **零內存洩漏**: 所有資源在 cleanup 標籤處釋放
2. **零殭屍設備**: 自動回滾防止孤立設備節點
3. **零遺漏**: UninstallDriver 找到所有匹配設備
4. **冪等性**: 安裝/卸載可以安全地重複執行
5. **原子性**: 安裝要麼完全成功，要麼完全回滾（無中間狀態）

---

## 📞 支持

如遇到問題：
1. 查看 `GetLastError()` 返回的詳細錯誤信息
2. 檢查 `driver_install_backup.txt` 了解安裝前狀態
3. 運行 `uninstall_driver.exe` 進行完整清理
4. 查閱本文檔的故障恢復指南

