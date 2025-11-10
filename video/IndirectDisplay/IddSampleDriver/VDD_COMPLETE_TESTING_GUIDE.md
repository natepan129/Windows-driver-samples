# VDD 完整功能測試指南

## 📋 概述

本文檔提供完整的 VDD SDK 功能測試指南，涵蓋所有核心功能和三個主要使用場景。

## 🔧 前置準備

### 1. 建置專案
```batch
# 建置 IddSampleDriver 和 vddctl
build_both_msbuild.bat
```

### 2. 確認文件存在
- `IddSampleDriver\build\bin\Release\vddctl.exe`
- `IddSampleDriver\x64\Release\IddSampleDriver.dll`
- `IddSampleDriver\x64\Release\IddSampleDriver.inf`
- `IddSampleDriver\x64\Release\IddSampleDriver\iddsampledriver.cat`

### 3. 管理員權限
部分測試（install/uninstall）需要管理員權限。請以管理員身份執行測試腳本。

## 🧪 自動化測試

### 快速測試（推薦）
```batch
# 執行完整自動化測試
test_all_vdd_features.bat
```

此腳本會自動測試所有功能，包括：
- ✅ 基本命令（version, help）
- ✅ SDK 管理（init, shutdown, status）
- ✅ 驅動管理（install, uninstall）
- ✅ 虛擬顯示管理（activate, deactivate, list）
- ✅ 顯示配置（setmode, setlocation, setprimary）

## 📝 手動測試步驟

### 階段 1: 基本命令測試

#### 1.1 版本信息
```batch
IddSampleDriver\build\bin\Release\vddctl.exe version
```
**預期結果**: 顯示 SDK 版本信息

#### 1.2 幫助信息
```batch
IddSampleDriver\build\bin\Release\vddctl.exe help
```
**預期結果**: 顯示所有可用命令和選項

### 階段 2: SDK 管理測試

#### 2.1 初始化 SDK
```batch
IddSampleDriver\build\bin\Release\vddctl.exe init
```
**預期結果**: SDK 初始化成功

#### 2.2 檢查狀態
```batch
IddSampleDriver\build\bin\Release\vddctl.exe status
```
**預期結果**: 顯示當前狀態（驅動安裝狀態、顯示激活狀態等）

#### 2.3 關閉 SDK
```batch
IddSampleDriver\build\bin\Release\vddctl.exe shutdown
```
**預期結果**: SDK 關閉成功

### 階段 3: 驅動管理測試（需要管理員權限）

#### 3.1 安裝驅動
```batch
IddSampleDriver\build\bin\Release\vddctl.exe install --path "IddSampleDriver\x64\Release\IddSampleDriver.inf"
```
**預期結果**: 
- 驅動安裝成功
- 三個虛擬監視器出現在 Device Manager
- 可能需要重啟（取決於系統狀態）

**驗證方法**:
```batch
# 檢查 Device Manager
devmgmt.msc
# 查看 "Monitors" 分類，應該看到 3 個 "IddSampleDriver Monitor"
```

#### 3.2 檢查安裝狀態
```batch
IddSampleDriver\build\bin\Release\vddctl.exe status
```
**預期結果**: 顯示 "Driver Installed: Yes"

#### 3.3 卸載驅動（可選）
```batch
IddSampleDriver\build\bin\Release\vddctl.exe uninstall
```
**預期結果**: 驅動卸載成功，虛擬監視器從 Device Manager 消失

### 階段 4: 虛擬顯示管理測試

#### 4.1 激活虛擬顯示
```batch
IddSampleDriver\build\bin\Release\vddctl.exe activate --name "VDD Test" --width 1920 --height 1080 --refresh 60
```
**預期結果**: 
- 虛擬顯示激活成功
- 顯示器出現在 Windows 顯示設置中
- 可以在顯示設置中看到新的顯示器

**驗證方法**:
```batch
# 打開顯示設置
ms-settings:display
# 或使用快捷鍵 Win+P 查看顯示器
```

#### 4.2 列出所有顯示器
```batch
IddSampleDriver\build\bin\Release\vddctl.exe list
```
**預期結果**: 列出所有顯示適配器和輸出，包括 VDD 適配器

#### 4.3 檢查激活狀態
```batch
IddSampleDriver\build\bin\Release\vddctl.exe status
```
**預期結果**: 顯示 "Display Active: Yes"

#### 4.4 停用虛擬顯示
```batch
IddSampleDriver\build\bin\Release\vddctl.exe deactivate
```
**預期結果**: 虛擬顯示停用，從 Windows 顯示設置中消失

### 階段 5: 顯示配置測試

#### 5.1 設置顯示模式
```batch
# 設置為 2560x1440@75Hz
IddSampleDriver\build\bin\Release\vddctl.exe setmode --index 0 --width 2560 --height 1440 --refresh 75

# 設置為 4K@60Hz
IddSampleDriver\build\bin\Release\vddctl.exe setmode --index 0 --width 3840 --height 2160 --refresh 60

# 設置為 1080p@144Hz（高刷新率）
IddSampleDriver\build\bin\Release\vddctl.exe setmode --index 0 --width 1920 --height 1080 --refresh 144
```
**預期結果**: 顯示模式成功切換，可以在顯示設置中驗證

#### 5.2 設置顯示位置
```batch
# 設置為第二個屏幕位置（右側）
IddSampleDriver\build\bin\Release\vddctl.exe setlocation --index 0 --x 1920 --y 0 --width 1920 --height 1080

# 設置為第三個屏幕位置
IddSampleDriver\build\bin\Release\vddctl.exe setlocation --index 0 --x 3840 --y 0 --width 1920 --height 1080
```
**預期結果**: 顯示器位置更新，可以在顯示設置中拖動驗證

#### 5.3 設置為主顯示器
```batch
IddSampleDriver\build\bin\Release\vddctl.exe setprimary --index 0
```
**⚠️ 重要警告**: 
- **如果您的系統只有一個物理螢幕，將 VDD 設置為主顯示器會導致物理螢幕變黑！**
- 這是因為主顯示器會切換到虛擬顯示器，而虛擬顯示器無法在物理螢幕上顯示
- **僅在有兩個或以上物理螢幕時才推薦使用此功能**
- 如果出現黑畫面，可以：
  1. 使用 `Win+P` 切換顯示模式
  2. 等待 30 秒後自動停用（如果使用測試腳本）
  3. 重啟電腦

**預期結果**: VDD 顯示器成為主顯示器，任務欄移動到該顯示器（僅在有兩個或以上物理螢幕時可見）

## 🎯 三個使用場景測試

### 場景 1: VDD 作為顯示代理（Display Proxy）

**用途**: 全屏 3D 應用在物理屏幕上運行，VDD 作為顯示代理

```batch
# 1. 初始化 SDK
IddSampleDriver\build\bin\Release\vddctl.exe init

# 2. 安裝驅動（如果尚未安裝）
IddSampleDriver\build\bin\Release\vddctl.exe install --path "IddSampleDriver\x64\Release\IddSampleDriver.inf"

# 3. 激活高刷新率虛擬顯示
IddSampleDriver\build\bin\Release\vddctl.exe activate --name "VDD Proxy" --width 1920 --height 1080 --refresh 120

# 4. 設置顯示位置（擴展模式，在物理屏幕右側）
IddSampleDriver\build\bin\Release\vddctl.exe setlocation --index 0 --x 1920 --y 0 --width 1920 --height 1080

# 5. 檢查狀態
IddSampleDriver\build\bin\Release\vddctl.exe status
```

**驗證**: 
- 虛擬顯示器出現在物理屏幕右側
- 可以在兩個顯示器之間拖動窗口
- 全屏應用可以選擇在虛擬顯示器上運行

### 場景 2: VDD 作為次要顯示器（Secondary Display）

**用途**: 全屏 3D 應用在 VDD 屏幕上運行，作為次要顯示器

```batch
# 1. 激活高解析度虛擬顯示
IddSampleDriver\build\bin\Release\vddctl.exe activate --name "VDD Remote" --width 2560 --height 1440 --refresh 90

# 2. 設置為擴展模式（在物理屏幕右側）
IddSampleDriver\build\bin\Release\vddctl.exe setlocation --index 0 --x 1920 --y 0 --width 2560 --height 1440

# 3. 檢查狀態
IddSampleDriver\build\bin\Release\vddctl.exe status
```

**驗證**:
- VDD 顯示器作為次要顯示器出現在右側
- 可以在 VDD 顯示器上運行全屏應用
- 物理屏幕保持可用

### 場景 3: VDD 作為主顯示器（Primary Display）

**⚠️ 重要警告**: 
- **此場景僅在有兩個或以上物理螢幕時推薦使用！**
- 如果只有一個物理螢幕，設置 VDD 為主顯示器會導致物理螢幕變黑
- 建議跳過此場景，除非您確定有多個物理螢幕

**用途**: VDD 作為主顯示器，用於遠程桌面或虛擬環境

```batch
# 1. 激活虛擬顯示
IddSampleDriver\build\bin\Release\vddctl.exe activate --name "VDD Primary" --width 1920 --height 1080 --refresh 90

# 2. 設置為主顯示器（⚠️ 僅在有兩個或以上物理螢幕時使用）
IddSampleDriver\build\bin\Release\vddctl.exe setprimary --index 0

# 3. 檢查狀態
IddSampleDriver\build\bin\Release\vddctl.exe status
```

**驗證**（僅在有兩個或以上物理螢幕時）:
- VDD 顯示器成為主顯示器
- 任務欄移動到 VDD 顯示器
- 登錄界面和系統 UI 顯示在 VDD 顯示器上

**如果出現黑畫面**:
1. 使用 `Win+P` 切換顯示模式（擴展/複製/僅第二屏幕）
2. 等待 30 秒後自動停用（如果使用測試腳本）
3. 重啟電腦

## 🔍 故障排除

### 問題 1: 驅動安裝後監視器未出現

**可能原因**:
- 需要重啟系統（某些情況下）
- 驅動未正確安裝
- Device Manager 需要刷新

**解決方法**:
```batch
# 1. 檢查 Device Manager
devmgmt.msc
# 查看 "Monitors" 分類

# 2. 手動掃描硬件變更
# 在 Device Manager 中: Action -> Scan for hardware changes

# 3. 檢查驅動狀態
IddSampleDriver\build\bin\Release\vddctl.exe status
```

### 問題 2: Activate 後顯示器未出現

**可能原因**:
- 設備未啟用
- 顯示拓撲未配置
- Windows 顯示服務需要刷新

**解決方法**:
```batch
# 1. 檢查狀態
IddSampleDriver\build\bin\Release\vddctl.exe status

# 2. 檢查 Device Manager 中的設備狀態
# 確保設備已啟用（無黃色感嘆號）

# 3. 手動刷新顯示設置
# Win+P 切換顯示模式，或打開 ms-settings:display
```

### 問題 3: 設置主顯示器後出現黑畫面

**原因**: 只有一個物理螢幕時，將 VDD 設置為主顯示器會導致物理螢幕變黑

**⚠️ 緊急恢復方法（黑畫面時）**:

#### 方法 1: 使用鍵盤快捷鍵切換顯示模式（推薦，最快）
1. **按 `Win + P`**（不需要看到屏幕）
2. 等待 1-2 秒
3. **按 `→`（右箭頭）一次**，然後**按 `Enter`**
   - 這會選擇 "擴展" 模式，讓物理顯示器重新顯示
4. 如果還是不行，再按 `Win + P`，然後按 `←`（左箭頭）一次，按 `Enter`
   - 這會選擇 "僅電腦屏幕" 模式

#### 方法 2: 使用命令行停用 VDD（需要提前準備）
如果您預先打開了命令行窗口（在另一個虛擬桌面或遠程連接）：
```batch
IddSampleDriver\build\bin\Release\vddctl.exe deactivate
```

#### 方法 3: 使用遠程桌面連接
如果您有遠程桌面連接：
- 通過遠程桌面連接到電腦
- 執行 `vddctl deactivate` 命令

#### 方法 4: 強制重啟電腦（最後手段）
如果以上方法都不行，使用以下方法重啟（**不需要看到屏幕**）：

**方法 4a: 使用鍵盤快捷鍵重啟**
1. 按 `Ctrl + Alt + Del`（可能需要等待幾秒）
2. 按 `Tab` 鍵 3 次（移動到電源按鈕）
3. 按 `Enter`（打開電源菜單）
4. 按 `R`（選擇重啟）

**方法 4b: 使用命令行重啟（如果已打開命令行）**
```batch
shutdown /r /t 0
```

**方法 4c: 使用物理按鈕**
- 按住電源按鈕 5-10 秒強制關機，然後重新開機
- ⚠️ 這是最後手段，可能導致數據丟失

#### 方法 5: 使用安全模式
1. 強制重啟電腦（方法 4）
2. 在啟動時按 `F8` 或 `Shift + F8` 進入安全模式
3. 在安全模式下，VDD 驅動不會加載，顯示器會恢復正常
4. 登錄後，可以卸載驅動或停用 VDD

**預防措施**: 
- ⚠️ **僅在有兩個或以上物理螢幕時使用 `setprimary` 命令**
- 測試腳本會自動提示並提供跳過選項
- 建議在測試前準備好遠程桌面連接或第二個顯示器
- 建議在測試前記住這些恢復方法

### 問題 4: SetMode 或 SetLocation 無效

**可能原因**:
- 顯示器未激活
- 參數無效
- Windows 顯示服務未響應

**解決方法**:
```batch
# 1. 確認顯示器已激活
IddSampleDriver\build\bin\Release\vddctl.exe status

# 2. 檢查參數是否有效
IddSampleDriver\build\bin\Release\vddctl.exe list

# 3. 重新激活顯示器
IddSampleDriver\build\bin\Release\vddctl.exe deactivate
IddSampleDriver\build\bin\Release\vddctl.exe activate --name "Test" --width 1920 --height 1080 --refresh 60
```

## 📊 測試檢查清單

### 基本功能
- [ ] version 命令正常
- [ ] help 命令正常
- [ ] init 命令正常
- [ ] shutdown 命令正常
- [ ] status 命令正常

### 驅動管理
- [ ] install 命令正常（管理員）
- [ ] uninstall 命令正常（管理員）
- [ ] 驅動安裝後出現在 Device Manager
- [ ] 驅動卸載後從 Device Manager 消失

### 虛擬顯示管理
- [ ] activate 命令正常
- [ ] deactivate 命令正常
- [ ] list 命令正常
- [ ] 激活後顯示器出現在 Windows 顯示設置
- [ ] 停用後顯示器從 Windows 顯示設置消失

### 顯示配置
- [ ] setmode 命令正常（多種解析度）
- [ ] setlocation 命令正常
- [ ] setprimary 命令正常
- [ ] 模式切換在顯示設置中可見
- [ ] 位置設置在顯示設置中可見
- [ ] 主顯示器切換成功

### 使用場景
- [ ] 場景 1: VDD 作為顯示代理
- [ ] 場景 2: VDD 作為次要顯示器
- [ ] 場景 3: VDD 作為主顯示器

## 📝 測試報告模板

```
測試日期: ___________
測試人員: ___________
系統信息: ___________

測試結果:
- 通過: ___ / ___
- 失敗: ___ / ___
- 跳過: ___ / ___

備註:
_________________________________
_________________________________
```

## 🚀 下一步

完成所有測試後，可以：
1. 記錄測試結果和發現的問題
2. 驗證三個使用場景是否符合需求
3. 測試邊界情況和錯誤處理
4. 進行性能測試（如果適用）

