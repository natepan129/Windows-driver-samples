# VDD SDK 命令行使用手冊

## 環境要求
- **管理員權限** - 所有 install/uninstall 命令都需要管理員權限
- 驅動文件路徑：`x64\Release\IddSampleDriver\IddSampleDriver.inf`

## 基本命令

### 1. 安裝驅動 (Install)
```powershell
# PowerShell (管理員)
.\build\bin\Release\vddctl.exe install x64\Release\IddSampleDriver\IddSampleDriver.inf

# CMD (管理員)
.\build\bin\Release\vddctl.exe install x64\Release\IddSampleDriver\IddSampleDriver.inf
```

**預期輸出：**
```
Installing driver...
Driver installed successfully
```

### 2. 卸載驅動 (Uninstall)
```powershell
# PowerShell (管理員)
.\build\bin\Release\vddctl.exe uninstall

# CMD (管理員)
.\build\bin\Release\vddctl.exe uninstall
```

**預期輸出：**
```
Uninstalling driver...
Driver uninstalled successfully.
```

### 3. 查看版本 (Version)
```powershell
# PowerShell / CMD
.\build\bin\Release\vddctl.exe version
```

**預期輸出：**
```
VDD SDK Version: 1.0.0
Build Date: [date]
```

### 4. 查看幫助 (Help)
```powershell
# PowerShell / CMD
.\build\bin\Release\vddctl.exe help

# 或
.\build\bin\Release\vddctl.exe --help
.\build\bin\Release\vddctl.exe -h
```

## 驗證命令

### 檢查驅動是否已安裝
```powershell
# PowerShell
Get-PnpDevice | Where-Object { $_.InstanceId -like '*IddSampleDriver*' } | Format-Table -AutoSize
```

**預期輸出（已安裝）：**
```
Status Class   FriendlyName           InstanceId               
------ -----   ------------           ----------               
OK     Display IddSampleDriver Device ROOT\IDDSAMPLEDRIVER\0000
```

### 檢查設備狀態
```powershell
# PowerShell
$device = Get-PnpDevice | Where-Object { $_.InstanceId -like '*IddSampleDriver*' }
if ($device) {
    Write-Host "Device Status: $($device.Status)" -ForegroundColor Green
    $prob = (Get-PnpDeviceProperty -InstanceId $device.InstanceId -KeyName 'DEVPKEY_Device_ProblemCode').Data
    if ($prob -eq 0) {
        Write-Host "ProblemCode: 0 (OK)" -ForegroundColor Green
    } else {
        Write-Host "ProblemCode: $prob (Error)" -ForegroundColor Red
    }
} else {
    Write-Host "Device not found" -ForegroundColor Yellow
}
```

## 完整測試流程

### 測試 Install + Uninstall
```powershell
# PowerShell (管理員)

Write-Host "`n=== Step 1: Check initial state ===" -ForegroundColor Cyan
Get-PnpDevice | Where-Object { $_.InstanceId -like '*IddSampleDriver*' }

Write-Host "`n=== Step 2: Install driver ===" -ForegroundColor Cyan
.\build\bin\Release\vddctl.exe install x64\Release\IddSampleDriver\IddSampleDriver.inf

Write-Host "`n=== Step 3: Verify installation ===" -ForegroundColor Cyan
Get-PnpDevice | Where-Object { $_.InstanceId -like '*IddSampleDriver*' } | Format-Table -AutoSize

Write-Host "`n=== Step 4: Check ProblemCode ===" -ForegroundColor Cyan
$device = Get-PnpDevice | Where-Object { $_.InstanceId -like '*IddSampleDriver*' }
if ($device) {
    $prob = (Get-PnpDeviceProperty -InstanceId $device.InstanceId -KeyName 'DEVPKEY_Device_ProblemCode').Data
    Write-Host "ProblemCode: $prob"
}

Write-Host "`n=== Step 5: Uninstall driver ===" -ForegroundColor Cyan
.\build\bin\Release\vddctl.exe uninstall

Write-Host "`n=== Step 6: Verify removal ===" -ForegroundColor Cyan
$device = Get-PnpDevice | Where-Object { $_.InstanceId -like '*IddSampleDriver*' }
if ($device) {
    Write-Host "Device still present!" -ForegroundColor Red
    $device | Format-Table -AutoSize
} else {
    Write-Host "Device successfully removed!" -ForegroundColor Green
}
```

### 一鍵測試腳本
將上述測試流程保存為 `test_vdd.ps1`，然後以管理員身份運行：
```powershell
# PowerShell (管理員)
.\test_vdd.ps1
```

## 常見錯誤處理

### 錯誤 1: "Administrator privileges required"
**原因：** 未以管理員身份運行  
**解決：** 右鍵 PowerShell/CMD → "以系統管理員身分執行"

### 錯誤 2: "Device already installed"
**原因：** 驅動已安裝，嘗試重複安裝  
**解決：** 先執行 `vddctl uninstall`

### 錯誤 3: "Failed to open INF file"
**原因：** INF 文件路徑不正確  
**解決：** 確認使用 `x64\Release\IddSampleDriver\IddSampleDriver.inf`

### 錯誤 4: "Failed to remove device: 5" (Access Denied)
**原因：** 權限不足  
**解決：** 確認以管理員身份運行

## Design Document 場景測試

根據設計文檔，有三個主要場景：

### 場景 1: VDD as Source (Display Proxy)
```powershell
# 1. 安裝驅動
.\build\bin\Release\vddctl.exe install x64\Release\IddSampleDriver\IddSampleDriver.inf

# 2. 啟用 VDD (未實現，待開發)
# .\build\bin\Release\vddctl.exe activate

# 3. 設置模式 (未實現，待開發)
# .\build\bin\Release\vddctl.exe setmode --width 1920 --height 1080
```

### 場景 2: VDD as Destination (Secondary Display)
```powershell
# 1. 安裝驅動
.\build\bin\Release\vddctl.exe install x64\Release\IddSampleDriver\IddSampleDriver.inf

# 2. 設置為次要顯示器 (未實現，待開發)
# .\build\bin\Release\vddctl.exe setlocation --position right-of-primary
```

### 場景 3: VDD as Destination (Primary Display)
```powershell
# 1. 安裝驅動
.\build\bin\Release\vddctl.exe install x64\Release\IddSampleDriver\IddSampleDriver.inf

# 2. 設置為主顯示器 (未實現，待開發)
# .\build\bin\Release\vddctl.exe setprimary
```

## 當前實現狀態

✅ **已實現：**
- `install` - 安裝驅動（含所有 GPT 強化功能）
- `uninstall` - 卸載驅動（含完整清理）
- `version` - 顯示版本信息
- `help` - 顯示幫助信息

⏳ **待實現：**
- `activate` / `deactivate` - 啟用/停用設備
- `setmode` - 設置顯示模式
- `setlocation` - 設置顯示位置
- `setprimary` - 設置為主顯示器
- `beginsession` / `endsession` - Session 管理
- `heartbeat` - 心跳保活



# VDD SDK 命令行使用手冊

## 環境要求
- **管理員權限** - 所有 install/uninstall 命令都需要管理員權限
- 驅動文件路徑：`x64\Release\IddSampleDriver\IddSampleDriver.inf`

## 基本命令

### 1. 安裝驅動 (Install)
```powershell
# PowerShell (管理員)
.\build\bin\Release\vddctl.exe install x64\Release\IddSampleDriver\IddSampleDriver.inf

# CMD (管理員)
.\build\bin\Release\vddctl.exe install x64\Release\IddSampleDriver\IddSampleDriver.inf
```

**預期輸出：**
```
Installing driver...
Driver installed successfully
```

### 2. 卸載驅動 (Uninstall)
```powershell
# PowerShell (管理員)
.\build\bin\Release\vddctl.exe uninstall

# CMD (管理員)
.\build\bin\Release\vddctl.exe uninstall
```

**預期輸出：**
```
Uninstalling driver...
Driver uninstalled successfully.
```

### 3. 查看版本 (Version)
```powershell
# PowerShell / CMD
.\build\bin\Release\vddctl.exe version
```

**預期輸出：**
```
VDD SDK Version: 1.0.0
Build Date: [date]
```

### 4. 查看幫助 (Help)
```powershell
# PowerShell / CMD
.\build\bin\Release\vddctl.exe help

# 或
.\build\bin\Release\vddctl.exe --help
.\build\bin\Release\vddctl.exe -h
```

## 驗證命令

### 檢查驅動是否已安裝
```powershell
# PowerShell
Get-PnpDevice | Where-Object { $_.InstanceId -like '*IddSampleDriver*' } | Format-Table -AutoSize
```

**預期輸出（已安裝）：**
```
Status Class   FriendlyName           InstanceId               
------ -----   ------------           ----------               
OK     Display IddSampleDriver Device ROOT\IDDSAMPLEDRIVER\0000
```

### 檢查設備狀態
```powershell
# PowerShell
$device = Get-PnpDevice | Where-Object { $_.InstanceId -like '*IddSampleDriver*' }
if ($device) {
    Write-Host "Device Status: $($device.Status)" -ForegroundColor Green
    $prob = (Get-PnpDeviceProperty -InstanceId $device.InstanceId -KeyName 'DEVPKEY_Device_ProblemCode').Data
    if ($prob -eq 0) {
        Write-Host "ProblemCode: 0 (OK)" -ForegroundColor Green
    } else {
        Write-Host "ProblemCode: $prob (Error)" -ForegroundColor Red
    }
} else {
    Write-Host "Device not found" -ForegroundColor Yellow
}
```

## 完整測試流程

### 測試 Install + Uninstall
```powershell
# PowerShell (管理員)

Write-Host "`n=== Step 1: Check initial state ===" -ForegroundColor Cyan
Get-PnpDevice | Where-Object { $_.InstanceId -like '*IddSampleDriver*' }

Write-Host "`n=== Step 2: Install driver ===" -ForegroundColor Cyan
.\build\bin\Release\vddctl.exe install x64\Release\IddSampleDriver\IddSampleDriver.inf

Write-Host "`n=== Step 3: Verify installation ===" -ForegroundColor Cyan
Get-PnpDevice | Where-Object { $_.InstanceId -like '*IddSampleDriver*' } | Format-Table -AutoSize

Write-Host "`n=== Step 4: Check ProblemCode ===" -ForegroundColor Cyan
$device = Get-PnpDevice | Where-Object { $_.InstanceId -like '*IddSampleDriver*' }
if ($device) {
    $prob = (Get-PnpDeviceProperty -InstanceId $device.InstanceId -KeyName 'DEVPKEY_Device_ProblemCode').Data
    Write-Host "ProblemCode: $prob"
}

Write-Host "`n=== Step 5: Uninstall driver ===" -ForegroundColor Cyan
.\build\bin\Release\vddctl.exe uninstall

Write-Host "`n=== Step 6: Verify removal ===" -ForegroundColor Cyan
$device = Get-PnpDevice | Where-Object { $_.InstanceId -like '*IddSampleDriver*' }
if ($device) {
    Write-Host "Device still present!" -ForegroundColor Red
    $device | Format-Table -AutoSize
} else {
    Write-Host "Device successfully removed!" -ForegroundColor Green
}
```

### 一鍵測試腳本
將上述測試流程保存為 `test_vdd.ps1`，然後以管理員身份運行：
```powershell
# PowerShell (管理員)
.\test_vdd.ps1
```

## 常見錯誤處理

### 錯誤 1: "Administrator privileges required"
**原因：** 未以管理員身份運行  
**解決：** 右鍵 PowerShell/CMD → "以系統管理員身分執行"

### 錯誤 2: "Device already installed"
**原因：** 驅動已安裝，嘗試重複安裝  
**解決：** 先執行 `vddctl uninstall`

### 錯誤 3: "Failed to open INF file"
**原因：** INF 文件路徑不正確  
**解決：** 確認使用 `x64\Release\IddSampleDriver\IddSampleDriver.inf`

### 錯誤 4: "Failed to remove device: 5" (Access Denied)
**原因：** 權限不足  
**解決：** 確認以管理員身份運行

## Design Document 場景測試

根據設計文檔，有三個主要場景：

### 場景 1: VDD as Source (Display Proxy)
```powershell
# 1. 安裝驅動
.\build\bin\Release\vddctl.exe install x64\Release\IddSampleDriver\IddSampleDriver.inf

# 2. 啟用 VDD (未實現，待開發)
# .\build\bin\Release\vddctl.exe activate

# 3. 設置模式 (未實現，待開發)
# .\build\bin\Release\vddctl.exe setmode --width 1920 --height 1080
```

### 場景 2: VDD as Destination (Secondary Display)
```powershell
# 1. 安裝驅動
.\build\bin\Release\vddctl.exe install x64\Release\IddSampleDriver\IddSampleDriver.inf

# 2. 設置為次要顯示器 (未實現，待開發)
# .\build\bin\Release\vddctl.exe setlocation --position right-of-primary
```

### 場景 3: VDD as Destination (Primary Display)
```powershell
# 1. 安裝驅動
.\build\bin\Release\vddctl.exe install x64\Release\IddSampleDriver\IddSampleDriver.inf

# 2. 設置為主顯示器 (未實現，待開發)
# .\build\bin\Release\vddctl.exe setprimary
```

## 當前實現狀態

✅ **已實現：**
- `install` - 安裝驅動（含所有 GPT 強化功能）
- `uninstall` - 卸載驅動（含完整清理）
- `version` - 顯示版本信息
- `help` - 顯示幫助信息

⏳ **待實現：**
- `activate` / `deactivate` - 啟用/停用設備
- `setmode` - 設置顯示模式
- `setlocation` - 設置顯示位置
- `setprimary` - 設置為主顯示器
- `beginsession` / `endsession` - Session 管理
- `heartbeat` - 心跳保活



