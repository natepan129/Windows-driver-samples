# VDD VirtualBox 完整部署指南

## 🎯 在 VirtualBox 中部署 VDD 的完整命令列方案

### 前提條件
- VirtualBox 虛擬機
- Windows 10/11 客體系統
- 管理員權限
- 已建置的 VDD 檔案

---

## 📋 步驟 1: 檢查環境

```powershell
# 檢查管理員權限
net session

# 檢查當前目錄
pwd

# 檢查 VDD 檔案
dir .\x64\Debug\IddSampleDriver\
```

---

## 📋 步驟 2: 準備驅動程式

```powershell
# 設定路徑變數
$VDD_DIR = "C:\Users\WDKRemoteUser\source\repos\IndirectDisplay"
$DRIVER_DIR = "$VDD_DIR\x64\Debug\IddSampleDriver"

# 檢查必要檔案
Test-Path "$DRIVER_DIR\IddSampleDriver.dll"
Test-Path "$DRIVER_DIR\IddSampleDriver.inf"
Test-Path "$DRIVER_DIR\iddsampledriver.cat"
```

---

## 📋 步驟 3: 安裝驅動程式

```powershell
# 方法 1: 使用 pnputil (推薦)
pnputil /add-driver "$DRIVER_DIR\IddSampleDriver.inf" /install

# 方法 2: 使用 devcon (如果可用)
# devcon install "$DRIVER_DIR\IddSampleDriver.inf" "ROOT\IddSampleDriver"

# 方法 3: 使用 PowerShell
Add-WindowsDriver -Path "$DRIVER_DIR" -Force
```

---

## 📋 步驟 4: 驗證安裝

```powershell
# 檢查驅動程式是否已安裝
pnputil /enum-drivers | Select-String "IddSampleDriver"

# 檢查裝置管理員
Get-PnpDevice | Where-Object {$_.FriendlyName -like "*Idd*"}

# 檢查 Windows 顯示設定
Get-WmiObject -Class Win32_VideoController | Select-Object Name, Status
```

---

## 📋 步驟 5: 啟用虛擬顯示器

```powershell
# 檢查顯示器設定
Get-WmiObject -Class Win32_DesktopMonitor

# 強制重新偵測顯示器
rundll32.exe user32.dll,UpdatePerUserSystemParameters

# 重新整理顯示器
$shell = New-Object -ComObject WScript.Shell
$shell.SendKeys([System.Windows.Forms.Keys]::F5)
```

---

## 📋 步驟 6: 測試虛擬顯示器

```powershell
# 執行範例應用程式
Start-Process "$VDD_DIR\x64\Debug\IddSampleApp.exe"

# 檢查虛擬顯示器是否出現
Get-WmiObject -Class Win32_DesktopMonitor | Where-Object {$_.Name -like "*Virtual*"}
```

---

## 🔧 故障排除命令

### 檢查驅動程式狀態
```powershell
# 檢查驅動程式簽名
Get-ChildItem "$DRIVER_DIR\*.cat" | Get-AuthenticodeSignature

# 檢查系統事件
Get-WinEvent -LogName System | Where-Object {$_.Message -like "*Idd*"} | Select-Object -First 5

# 檢查裝置管理員錯誤
Get-PnpDevice | Where-Object {$_.Status -ne "OK"} | Select-Object FriendlyName, Status, Problem
```

### 重新安裝驅動程式
```powershell
# 卸載舊驅動程式
pnputil /delete-driver "IddSampleDriver.inf" /uninstall

# 清理驅動程式快取
pnputil /delete-driver "IddSampleDriver.inf" /force

# 重新安裝
pnputil /add-driver "$DRIVER_DIR\IddSampleDriver.inf" /install
```

### 修復驅動程式簽名
```powershell
# 禁用驅動程式簽名驗證 (需要重啟)
bcdedit /set testsigning on

# 重新啟用驅動程式簽名驗證
bcdedit /set testsigning off
```

---

## 🚀 一鍵部署腳本

```powershell
# 完整的 VDD 部署腳本
$VDD_DIR = "C:\Users\WDKRemoteUser\source\repos\IndirectDisplay"
$DRIVER_DIR = "$VDD_DIR\x64\Debug\IddSampleDriver"

Write-Host "=== VDD VirtualBox 部署腳本 ===" -ForegroundColor Green

# 檢查管理員權限
if (-NOT ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole] "Administrator")) {
    Write-Host "需要管理員權限！" -ForegroundColor Red
    exit 1
}

# 檢查檔案
if (-not (Test-Path "$DRIVER_DIR\IddSampleDriver.dll")) {
    Write-Host "找不到驅動程式 DLL！" -ForegroundColor Red
    exit 1
}

# 安裝驅動程式
Write-Host "安裝 VDD 驅動程式..." -ForegroundColor Yellow
pnputil /add-driver "$DRIVER_DIR\IddSampleDriver.inf" /install

# 驗證安裝
Write-Host "驗證安裝..." -ForegroundColor Yellow
$result = pnputil /enum-drivers | Select-String "IddSampleDriver"
if ($result) {
    Write-Host "VDD 驅動程式安裝成功！" -ForegroundColor Green
} else {
    Write-Host "VDD 驅動程式安裝失敗！" -ForegroundColor Red
}

# 測試虛擬顯示器
Write-Host "測試虛擬顯示器..." -ForegroundColor Yellow
Start-Process "$VDD_DIR\x64\Debug\IddSampleApp.exe"

Write-Host "部署完成！請檢查 Windows 顯示設定。" -ForegroundColor Green
```

---

## 📊 監控和診斷

### 即時監控
```powershell
# 監控驅動程式事件
Get-WinEvent -LogName System -MaxEvents 10 | Where-Object {$_.Message -like "*Idd*"}

# 監控顯示器變更
Register-WmiEvent -Query "SELECT * FROM Win32_DesktopMonitor" -Action {Write-Host "顯示器變更: $($Event.SourceEventArgs.NewEvent)"}
```

### 效能監控
```powershell
# 檢查記憶體使用
Get-Process | Where-Object {$_.ProcessName -like "*Idd*"} | Select-Object ProcessName, WorkingSet

# 檢查 CPU 使用
Get-Counter "\Process(IddSampleApp)\% Processor Time"
```

---

## 🎯 三個目標場景的實現

### 場景 1: VDD 作為代理 (Source)
```powershell
# 啟用虛擬顯示器
Start-Process "$VDD_DIR\x64\Debug\IddSampleApp.exe"

# 設定顯示器為延伸模式
$shell = New-Object -ComObject WScript.Shell
$shell.SendKeys([System.Windows.Forms.Keys]::F5)
```

### 場景 2: VDD 作為次要顯示器 (Target)
```powershell
# 檢查次要顯示器
Get-WmiObject -Class Win32_DesktopMonitor | Where-Object {$_.Name -like "*Virtual*"}
```

### 場景 3: VDD 作為主要顯示器 (Target)
```powershell
# 設定主要顯示器
$shell = New-Object -ComObject WScript.Shell
$shell.SendKeys([System.Windows.Forms.Keys]::F5)
```

---

## ⚠️ VirtualBox 特殊注意事項

1. **虛擬機設定**: 確保啟用 3D 加速
2. **記憶體分配**: 建議至少 4GB RAM
3. **顯示卡設定**: 啟用硬體加速
4. **網路設定**: 確保 NAT 或橋接模式正常

---

## 🔧 常見問題解決

### 問題 1: 驅動程式安裝失敗
```powershell
# 檢查驅動程式簽名
Get-AuthenticodeSignature "$DRIVER_DIR\IddSampleDriver.dll"

# 禁用簽名驗證
bcdedit /set testsigning on
```

### 問題 2: 虛擬顯示器不顯示
```powershell
# 強制重新偵測
rundll32.exe user32.dll,UpdatePerUserSystemParameters

# 重新整理顯示器
$shell = New-Object -ComObject WScript.Shell
$shell.SendKeys([System.Windows.Forms.Keys]::F5)
```

### 問題 3: 效能問題
```powershell
# 檢查記憶體使用
Get-Process | Where-Object {$_.ProcessName -like "*Idd*"}

# 檢查 CPU 使用
Get-Counter "\Process(IddSampleApp)\% Processor Time"
```

---

## 📝 總結

這個完整的命令列方案讓您可以在 VirtualBox 中直接部署 VDD，無需任何 GUI 工具或複雜的建置過程。所有操作都可以通過 PowerShell 命令列完成。
