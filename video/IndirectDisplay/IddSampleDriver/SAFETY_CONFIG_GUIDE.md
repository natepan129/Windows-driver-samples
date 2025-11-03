# VDD SDK 安全配置指南

## 概述

VDD SDK 提供**可配置的安全機制**，讓開發者根據不同場景選擇合適的保護級別。

---

## 🎛️ 配置選項

### SdkConfig 新增安全選項

```cpp
struct SdkConfig {
    // ... 其他配置 ...
    
    // 安裝安全選項
    bool enableStateBackup = false;           // 安裝前備份系統狀態
    std::wstring backupFilePath = L"vdd_install_backup.txt";  // 備份文件路徑
    bool autoRollbackOnFailure = true;        // 安裝失敗時自動回滾
};
```

---

## 📊 配置矩陣

| 場景 | enableStateBackup | autoRollbackOnFailure | 說明 |
|------|-------------------|----------------------|------|
| **生產環境（推薦）** | `false` | `true` | 輕量級，自動恢復 |
| **開發/測試** | `true` | `true` | 完整保護，便於調試 |
| **自動化部署** | `false` | `true` | 高效，無人值守 |
| **手動測試** | `true` | `false` | 保留失敗狀態供分析 |

---

## 💡 使用示例

### 示例 1: 生產環境（默認推薦）

```cpp
#include "vddsdk.h"

int main() {
    // 默認配置：輕量級 + 自動回滾
    vdd::SdkConfig config;
    config.autoRollbackOnFailure = true;   // 默認已啟用
    config.enableStateBackup = false;       // 默認關閉（輕量級）
    
    vdd::Status status = vdd::Initialize(config);
    if (status != vdd::Status::Ok) {
        std::cout << "Init failed: " << vdd::GetLastError() << std::endl;
        return 1;
    }
    
    // 安裝驅動
    status = vdd::InstallDriver(L"C:\\Drivers\\IddSampleDriver.inf");
    if (status != vdd::Status::Ok) {
        // 自動回滾已執行，設備已清理
        std::cout << "Install failed: " << vdd::GetLastError() << std::endl;
        return 1;
    }
    
    std::cout << "Driver installed successfully!" << std::endl;
    return 0;
}
```

**優點**：
- ✅ 輕量級，無額外 I/O
- ✅ 自動恢復，零殘留
- ✅ 適合大規模部署

---

### 示例 2: 開發/測試環境（完整保護）

```cpp
#include "vddsdk.h"
#include <iostream>

int main() {
    // 完整保護配置
    vdd::SdkConfig config;
    config.enableStateBackup = true;        // 啟用狀態備份
    config.backupFilePath = L"my_backup.txt";
    config.autoRollbackOnFailure = true;    // 啟用自動回滾
    
    vdd::Status status = vdd::Initialize(config);
    
    // 安裝驅動
    status = vdd::InstallDriver(L"driver.inf");
    if (status != vdd::Status::Ok) {
        std::cout << "Install failed: " << vdd::GetLastError() << std::endl;
        std::cout << "Check backup file: " << "my_backup.txt" << std::endl;
        return 1;
    }
    
    return 0;
}
```

**優點**：
- ✅ 完整保護，失敗前狀態可追溯
- ✅ 便於故障分析
- ✅ 自動回滾仍然生效

**備份文件示例**：
```
=== VDD Installation State Backup ===
Timestamp: Thu Oct 30 14:23:45 2025

=== Existing Display Devices ===
  Device: PCI\VEN_1414&DEV_008C&SUBSYS_00000000&REV_00\3&267A616A&0&08
  Description: Microsoft Basic Display Adapter

  Device: ROOT\BASICDISPLAY\0000
  Description: Microsoft Basic Display Driver
```

---

### 示例 3: 手動測試（禁用自動回滾）

```cpp
int main() {
    vdd::SdkConfig config;
    config.enableStateBackup = true;        // 保存狀態
    config.autoRollbackOnFailure = false;   // 禁用自動回滾
    
    vdd::Initialize(config);
    
    vdd::Status status = vdd::InstallDriver(L"driver.inf");
    if (status != vdd::Status::Ok) {
        // 失敗的設備仍保留，可以手動分析
        std::cout << "Install failed but device kept for analysis" << std::endl;
        std::cout << "Use: Get-PnpDevice | Where-Object { $_.FriendlyName -like '*IddSampleDriver*' }" << std::endl;
        
        // 需要時手動清理
        // vdd::UninstallDriver();
        return 1;
    }
    
    return 0;
}
```

**適用場景**：
- 需要檢查失敗時的設備狀態
- 調試驅動安裝問題
- 測試設備管理器中的行為

---

## 🔍 配置決策樹

```
需要備份安裝前狀態嗎？
├─ 是 → enableStateBackup = true
│   └─ 需要自動回滾嗎？
│       ├─ 是 → autoRollbackOnFailure = true  ✅ 完整保護
│       └─ 否 → autoRollbackOnFailure = false  🔍 調試模式
│
└─ 否 → enableStateBackup = false
    └─ 需要自動回滾嗎？
        ├─ 是 → autoRollbackOnFailure = true  ✅ 生產推薦
        └─ 否 → autoRollbackOnFailure = false  ⚠️ 危險！
```

---

## ⚠️ 重要提示

### 關於 enableStateBackup

**優點**：
- ✅ 可追溯安裝前狀態
- ✅ 便於故障分析
- ✅ 輔助手動恢復

**缺點**：
- ❌ 額外 I/O 開銷（枚舉設備）
- ❌ 需要寫入權限
- ❌ 不適合高頻安裝場景

**建議**：
- 開發/測試：**啟用**
- 生產環境：**關閉**（自動回滾已足夠）

### 關於 autoRollbackOnFailure

**永遠保持啟用** ✅

除非你需要：
- 檢查失敗時的設備狀態
- 調試驅動安裝流程
- 測試特定錯誤場景

**禁用風險**：
- ⚠️ 可能留下"殭屍設備"
- ⚠️ 需要手動清理
- ⚠️ 影響後續安裝

---

## 🧪 測試配置

### 單元測試配置

```cpp
// test_install.cpp
TEST(InstallDriver, WithBackupAndRollback) {
    vdd::SdkConfig config;
    config.enableStateBackup = true;
    config.backupFilePath = L"test_backup.txt";
    config.autoRollbackOnFailure = true;
    
    vdd::Initialize(config);
    
    // 使用無效 INF 測試回滾
    vdd::Status status = vdd::InstallDriver(L"invalid.inf");
    EXPECT_EQ(status, vdd::Status::InvalidArg);
    
    // 驗證備份文件已創建
    EXPECT_TRUE(std::filesystem::exists("test_backup.txt"));
    
    // 驗證設備已回滾
    // ... 驗證邏輯 ...
}
```

---

## 📝 最佳實踐

### 1. 生產環境部署

```cpp
vdd::SdkConfig config;
config.autoRollbackOnFailure = true;   // ✅ 必須啟用
config.enableStateBackup = false;      // ✅ 關閉以提高性能

// 錯誤處理
if (vdd::InstallDriver(infPath) != vdd::Status::Ok) {
    log_error(vdd::GetLastError());
    // 自動回滾已完成，直接返回錯誤
    return ERROR_INSTALL_FAILED;
}
```

### 2. 持續集成 (CI)

```cpp
vdd::SdkConfig config;
config.autoRollbackOnFailure = true;
config.enableStateBackup = true;       // ✅ CI 環境可啟用（便於調試）
config.backupFilePath = L"ci_backup_" + std::to_wstring(build_id) + L".txt";
```

### 3. 用戶安裝程序

```cpp
vdd::SdkConfig config;
config.autoRollbackOnFailure = true;
config.enableStateBackup = user_preference;  // 讓用戶選擇

if (config.enableStateBackup) {
    ShowMessage("System state will be backed up before installation");
}
```

---

## 🔄 與獨立工具的對比

| 特性 | vddsdk.dll API | install_driver.exe |
|------|----------------|-------------------|
| **狀態備份** | ✅ 可配置 | ✅ 永遠啟用 |
| **自動回滾** | ✅ 可配置 | ✅ 永遠啟用 |
| **用戶確認** | ❌ 不支持 | ✅ 互動式 |
| **適用場景** | 程式化控制 | 手動測試 |

---

## 🛠️ vddctl 命令行工具集成

`vddctl.exe` 可以通過命令行參數控制配置：

```batch
REM 標準安裝（默認配置）
vddctl install driver.inf

REM 帶備份的安裝
vddctl install --backup driver.inf

REM 禁用自動回滾（調試用）
vddctl install --no-rollback driver.inf

REM 完整保護
vddctl install --backup --backup-file=my_backup.txt driver.inf
```

**實現建議**：
```cpp
// vddctl.cpp
vdd::SdkConfig config;

if (args.has("backup")) {
    config.enableStateBackup = true;
    config.backupFilePath = args.get("backup-file", L"vdd_backup.txt");
}

if (args.has("no-rollback")) {
    config.autoRollbackOnFailure = false;
}

vdd::Initialize(config);
vdd::InstallDriver(args.get("inf"));
```

---

## 📞 故障排除

### 問題：備份文件創建失敗

**原因**：無寫入權限或磁盤空間不足

**解決**：
```cpp
config.enableStateBackup = true;
config.backupFilePath = L"C:\\Temp\\backup.txt";  // 使用有權限的路徑
```

### 問題：需要查看失敗時的設備狀態

**解決**：臨時禁用自動回滾
```cpp
config.autoRollbackOnFailure = false;
// 安裝失敗後手動檢查：
// Get-PnpDevice | Where-Object { $_.FriendlyName -like "*IddSampleDriver*" }
```

### 問題：高頻安裝導致性能問題

**解決**：關閉狀態備份
```cpp
config.enableStateBackup = false;  // 減少 I/O
```

---

## ✅ 總結

### 推薦配置

**生產環境 🌐**：
```cpp
config.enableStateBackup = false;
config.autoRollbackOnFailure = true;
```

**開發環境 🔧**：
```cpp
config.enableStateBackup = true;
config.autoRollbackOnFailure = true;
```

**調試模式 🐛**：
```cpp
config.enableStateBackup = true;
config.autoRollbackOnFailure = false;  // 僅在需要分析失敗狀態時
```

### 核心原則

1. **永遠保持 autoRollbackOnFailure = true**（除非調試）
2. **生產環境關閉 enableStateBackup**（性能優先）
3. **開發環境啟用 enableStateBackup**（便於追溯）
4. **不支持用戶確認**（保持 API 自動化）

---

## 📚 相關文檔

- `SAFETY_MECHANISMS.md` - 安全機制詳細說明
- `vddsdk.h` - API 參考
- `DRIVER_INSTALL_GUIDE.md` - 獨立工具使用指南



