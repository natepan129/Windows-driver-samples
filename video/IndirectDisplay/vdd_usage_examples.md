# VDD 命令列使用範例

## 🚀 快速開始

### 1. 建置專案
```bash
# 執行建置腳本
.\build_vdd.bat
```

### 2. 部署 VDD
```bash
# 以管理員身份執行部署腳本
.\deploy_vdd.bat
```

## 📋 完整命令列範例

### 基本操作

```powershell
# 初始化 VDD SDK
.\x64\Debug\vddctl.exe init

# 檢查狀態
.\x64\Debug\vddctl.exe status

# 查看版本
.\x64\Debug\vddctl.exe version

# 查看幫助
.\x64\Debug\vddctl.exe help
```

### 驅動程式管理

```powershell
# 安裝驅動程式 (需要管理員權限)
.\x64\Debug\vddctl.exe install --path ".\x64\Debug\IddSampleDriver\IddSampleDriver.inf"

# 卸載驅動程式 (需要管理員權限)
.\x64\Debug\vddctl.exe uninstall

# 檢查驅動程式是否已安裝
.\x64\Debug\vddctl.exe status
```

### 虛擬顯示器管理

#### 場景 1: VDD 作為代理 (Source)
```powershell
# 啟用虛擬顯示器作為代理
.\x64\Debug\vddctl.exe activate --name "VDD Proxy" --width 1920 --height 1080 --refresh 90 --count 1

# 設定位置 (放在主顯示器右側)
.\x64\Debug\vddctl.exe setlocation --index 0 --x 3840 --y 0 --width 1920 --height 1080

# 檢查狀態
.\x64\Debug\vddctl.exe list
```

#### 場景 2: VDD 作為次要顯示器 (Target)
```powershell
# 啟用虛擬顯示器作為次要顯示器
.\x64\Debug\vddctl.exe activate --name "VDD Remote" --width 2560 --height 1440 --refresh 90 --count 1

# 設定位置
.\x64\Debug\vddctl.exe setlocation --index 0 --x 3840 --y 0 --width 2560 --height 1440

# 不設定為主要顯示器，保持次要顯示器狀態
```

#### 場景 3: VDD 作為主要顯示器 (Target)
```powershell
# 啟用虛擬顯示器
.\x64\Debug\vddctl.exe activate --name "VDD Primary" --width 1920 --height 1080 --refresh 90 --count 1

# 設定為主要顯示器
.\x64\Debug\vddctl.exe setprimary --index 0

# 設定位置
.\x64\Debug\vddctl.exe setlocation --index 0 --x 0 --y 0 --width 1920 --height 1080
```

### 顯示器配置

```powershell
# 變更顯示器模式
.\x64\Debug\vddctl.exe setmode --index 0 --width 2560 --height 1440 --refresh 75

# 移動顯示器位置
.\x64\Debug\vddctl.exe setlocation --index 0 --x 1920 --y 0 --width 1920 --height 1080

# 設定主要顯示器
.\x64\Debug\vddctl.exe setprimary --index 0

# 列出所有顯示器
.\x64\Debug\vddctl.exe list
```

### 進階功能

```powershell
# 啟用 HDR 支援
.\x64\Debug\vddctl.exe activate --name "VDD HDR" --width 3840 --height 2160 --refresh 60 --hdr

# 啟用立體 3D 支援
.\x64\Debug\vddctl.exe activate --name "VDD 3D" --width 1920 --height 1080 --refresh 90 --stereo

# 建立多個虛擬顯示器
.\x64\Debug\vddctl.exe activate --name "VDD Multi" --width 1920 --height 1080 --refresh 60 --count 3
```

### 故障排除

```powershell
# 停用所有虛擬顯示器
.\x64\Debug\vddctl.exe deactivate

# 關閉 SDK
.\x64\Debug\vddctl.exe shutdown

# 重新初始化
.\x64\Debug\vddctl.exe init

# 檢查錯誤訊息
.\x64\Debug\vddctl.exe status
```

## 🔧 自動化腳本範例

### 一鍵部署腳本
```batch
@echo off
echo 部署 VDD 虛擬顯示器...

REM 初始化
.\x64\Debug\vddctl.exe init

REM 啟用虛擬顯示器
.\x64\Debug\vddctl.exe activate --name "VDD XR" --width 1920 --height 1080 --refresh 90

REM 設定位置
.\x64\Debug\vddctl.exe setlocation --index 0 --x 3840 --y 0 --width 1920 --height 1080

REM 檢查狀態
.\x64\Debug\vddctl.exe status

echo 部署完成！
pause
```

### 清理腳本
```batch
@echo off
echo 清理 VDD 虛擬顯示器...

REM 停用虛擬顯示器
.\x64\Debug\vddctl.exe deactivate

REM 關閉 SDK
.\x64\Debug\vddctl.exe shutdown

echo 清理完成！
pause
```

## 📊 狀態檢查

```powershell
# 檢查完整狀態
.\x64\Debug\vddctl.exe status

# 列出所有顯示器
.\x64\Debug\vddctl.exe list

# 檢查特定顯示器模式
.\x64\Debug\vddctl.exe getmode --index 0

# 檢查特定顯示器位置
.\x64\Debug\vddctl.exe getlocation --index 0
```

## ⚠️ 注意事項

1. **管理員權限**: 安裝/卸載驅動程式需要管理員權限
2. **系統相容性**: 需要 Windows 10 2004+ 或 Windows 11
3. **驅動程式簽名**: 開發環境可能需要禁用驅動程式簽名驗證
4. **防火牆設定**: 可能需要允許 VDD 服務通過防火牆

## 🐛 常見問題

### Q: 驅動程式安裝失敗
```powershell
# 檢查驅動程式檔案是否存在
dir .\x64\Debug\IddSampleDriver\

# 手動安裝驅動程式
pnputil /add-driver ".\x64\Debug\IddSampleDriver\IddSampleDriver.inf" /install
```

### Q: 虛擬顯示器無法啟用
```powershell
# 檢查 SDK 狀態
.\x64\Debug\vddctl.exe status

# 重新初始化
.\x64\Debug\vddctl.exe shutdown
.\x64\Debug\vddctl.exe init
```

### Q: 顯示器位置設定失敗
```powershell
# 檢查顯示器索引
.\x64\Debug\vddctl.exe list

# 確保顯示器已啟用
.\x64\Debug\vddctl.exe status
```
