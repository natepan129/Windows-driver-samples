# 驅動程式安裝工具使用指南

## 📦 工具列表

| 工具 | 說明 | 功能 |
|------|------|------|
| `install_driver.exe` | 安裝驅動程式（帶回滾） | ✅ 自動安裝<br>✅ 狀態備份<br>✅ 失敗自動回滾<br>✅ 驗證安裝 |
| `uninstall_driver.exe` | 卸載驅動程式 | ✅ 完整移除設備<br>✅ 清理驅動程式存儲區<br>✅ 清理註冊表<br>✅ 驗證卸載 |

## 🚀 快速開始

### 安裝驅動程式

```batch
# 以管理員身份運行
install_driver.exe
```

或指定 INF 文件路徑：
```batch
install_driver.exe path\to\driver.inf
```

**安裝過程：**
1. 💾 備份當前系統狀態到 `driver_install_backup.txt`
2. 🔨 創建虛擬顯示設備節點
3. 📝 註冊設備到系統
4. 💿 安裝驅動程式文件
5. ✅ 驗證安裝成功

**如果安裝失敗：**
- 工具會詢問是否執行回滾
- 選擇 **Y** 將自動撤銷所有更改
- 系統將恢復到安裝前的狀態

### 卸載驅動程式

```batch
# 以管理員身份運行
uninstall_driver.exe
```

**卸載過程：**
1. 🔍 搜尋所有 IddSampleDriver 設備
2. 🗑️ 從設備管理器移除設備
3. 🧹 從驅動程式存儲區刪除驅動
4. 🔧 清理註冊表項目
5. ✅ 驗證卸載完成

## 📋 安裝檢查清單

### 安裝前
- [ ] 以管理員身份運行
- [ ] INF 文件在當前目錄或指定路徑
- [ ] 關閉所有可能使用顯示驅動的應用程式

### 安裝後驗證
```powershell
# 檢查設備管理器
Get-PnpDevice | Where-Object { $_.FriendlyName -like "*IddSampleDriver*" }

# 檢查驅動程式存儲區
pnputil /enum-drivers | findstr /i "iddsampledriver"

# 檢查註冊表
Test-Path "HKLM:\SYSTEM\CurrentControlSet\Enum\ROOT\IddSampleDriver"
```

## 🔄 回滾機制

### 自動回滾（安裝失敗時）
當安裝過程中任何步驟失敗，工具會詢問是否回滾：

```
❌ 安裝失敗！

是否要回滾更改? (Y/N): Y

========================================
執行回滾操作...
========================================

[回滾] 移除已註冊的設備...
  ✓ 設備已移除
[回滾] 清理註冊表項目...
  完成

✓ 回滾成功
```

### 手動卸載
如果需要完全移除驅動：

```batch
uninstall_driver.exe
```

## 📁 備份文件

### driver_install_backup.txt
安裝前自動創建的系統狀態備份：

```ini
[InstallState]
Timestamp=2025-01-15 10:30:45

[ExistingDevices]
Device0_ID=PCI\VEN_80EE&DEV_BEEF...
Device0_Desc=VirtualBox Graphics Adapter
DeviceCount=1
```

**用途：**
- 記錄安裝前的顯示設備狀態
- 提供回滾參考
- 故障排除依據

## 🛠️ 重新編譯工具

如果需要修改源代碼：

```batch
# 編譯所有工具
build_install_tools.bat
```

**源文件：**
- `setupapi_install_with_rollback.cpp` - 安裝工具（帶回滾）
- `setupapi_uninstall.cpp` - 卸載工具

## ⚠️ 常見問題

### 1. 安裝失敗：ERROR_NO_MORE_ITEMS (259)
**原因：** 設備不存在，UpdateDriverForPlugAndPlayDevices 找不到設備

**解決：** 使用帶回滾的工具會自動創建設備節點

### 2. 安裝失敗：ERROR 1004
**原因：** INF 文件格式問題或路徑錯誤

**解決：**
- 檢查 INF 文件路徑是否正確
- 確認 INF 文件格式正確
- 使用完整路徑

### 3. 卸載後設備仍然存在
**原因：** 系統緩存或服務未更新

**解決：**
```batch
# 重新掃描硬體
pnputil /scan-devices

# 或重新啟動系統
shutdown /r /t 0
```

### 4. 需要管理員權限
**錯誤：** "ERROR: Administrator privileges required"

**解決：**
- 右鍵程序 → 以管理員身份運行
- 或在管理員命令提示符中執行

## 🔍 進階操作

### 查看詳細日誌
工具會輸出詳細的操作日誌，可以重定向到文件：

```batch
install_driver.exe > install_log.txt 2>&1
uninstall_driver.exe > uninstall_log.txt 2>&1
```

### 批次操作
創建批處理腳本自動化安裝/卸載：

```batch
@echo off
echo 正在安裝驅動程式...
install_driver.exe IddSampleDriver_Fixed.inf

if %ERRORLEVEL% EQU 0 (
    echo 安裝成功
) else (
    echo 安裝失敗，執行清理...
    uninstall_driver.exe
)
```

### 與 VDD SDK 集成
安裝完成後，可以使用 VDD SDK 控制虛擬顯示：

```cpp
#include "vddsdk.h"

// 初始化 SDK
vdd::Initialize();

// 激活虛擬顯示
vdd::VirtualDisplayDesc desc;
desc.name = "My Virtual Display";
desc.preferredMode.width = 1920;
desc.preferredMode.height = 1080;
vdd::Activate(desc, 1);
```

## 📞 支援

如果遇到問題：

1. 檢查 `driver_install_backup.txt` 了解安裝前狀態
2. 查看工具輸出的錯誤訊息和錯誤碼
3. 在設備管理器中檢查設備狀態
4. 嘗試運行 `uninstall_driver.exe` 完全清理後重新安裝

## 📝 版本歷史

### v1.0 (2025-01)
- ✅ SetupAPI 驅動安裝
- ✅ 自動回滾機制
- ✅ 狀態備份功能
- ✅ 完整卸載工具
- ✅ 詳細錯誤報告

---

**安全提示：** 安裝和卸載驅動程式需要管理員權限。請確保從可信來源獲取驅動程式文件。

