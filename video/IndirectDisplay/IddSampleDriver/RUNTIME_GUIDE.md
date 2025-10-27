# VDD SDK - 運行指南

## 🚀 **快速開始**

### **前置要求**
- Windows 10/11 (x64)
- 管理員權限
- Visual Studio 2019+ 或 MSBuild
- Windows Driver Kit (WDK)

### **建置環境**
```bash
# 檢查建置工具
where cl
where msbuild
where cmake

# 如果沒有，需要安裝 Visual Studio 或 Build Tools
```

## 📦 **安裝步驟**

### **步驟 1: 獲取代碼**
```bash
git clone <repository-url>
cd IddSampleDriver
```

### **步驟 2: 建置項目**
```bash
# 使用 CMake
mkdir build
cd build
cmake ..
cmake --build . --config Release

# 或使用 Visual Studio
# 打開 IddSampleDriver.sln
# 選擇 Release 配置
# 建置解決方案
```

### **步驟 3: 安裝驅動**
```bash
# 以管理員身份運行命令提示符
# 導航到項目目錄
cd C:\path\to\IddSampleDriver

# 安裝驅動
pnputil /add-driver "IddSampleDriver_Fixed.inf" /install
```

## 🎮 **使用方法**

### **基本命令**

#### **1. 初始化 SDK**
```bash
build\bin\Release\vddctl.exe init
```

#### **2. 檢查狀態**
```bash
build\bin\Release\vddctl.exe status
```

#### **3. 激活虛擬顯示**
```bash
build\bin\Release\vddctl.exe activate --name "VDD Test" --width 1920 --height 1080 --refresh 60
```

#### **4. 設置顯示模式**
```bash
build\bin\Release\vddctl.exe setmode --index 0 --width 1920 --height 1080 --refresh 60
```

#### **5. 設置顯示位置**
```bash
build\bin\Release\vddctl.exe setlocation --index 0 --x 1920 --y 0 --width 1920 --height 1080
```

#### **6. 設置主顯示器**
```bash
build\bin\Release\vddctl.exe setprimary --index 0
```

#### **7. 列出顯示器**
```bash
build\bin\Release\vddctl.exe list
```

#### **8. 停用虛擬顯示**
```bash
build\bin\Release\vddctl.exe deactivate
```

#### **9. 關閉 SDK**
```bash
build\bin\Release\vddctl.exe shutdown
```

## 🎯 **使用場景**

### **場景 1: VDD 作為顯示代理**

**用途**: 全屏 3D 在任何物理屏幕上，VDD 作為顯示代理

```bash
# 初始化
build\bin\Release\vddctl.exe init

# 安裝驅動（如果未安裝）
build\bin\Release\vddctl.exe install --path "IddSampleDriver_Fixed.inf"

# 激活虛擬顯示
build\bin\Release\vddctl.exe activate --name "VDD Proxy" --width 1920 --height 1080 --refresh 120

# 設置位置（在第二個屏幕）
build\bin\Release\vddctl.exe setlocation --index 0 --x 3840 --y 0 --width 1920 --height 1080

# 檢查狀態
build\bin\Release\vddctl.exe status
```

### **場景 2: VDD 作為次要顯示器**

**用途**: 全屏 3D 在 VDD 屏幕上作為次要顯示器

```bash
# 激活虛擬顯示
build\bin\Release\vddctl.exe activate --name "VDD Remote" --width 2560 --height 1440 --refresh 90

# 設置位置
build\bin\Release\vddctl.exe setlocation --index 0 --x 3840 --y 0 --width 2560 --height 1440

# 檢查狀態
build\bin\Release\vddctl.exe status
```

### **場景 3: VDD 作為主顯示器**

**用途**: 全屏 3D 在 VDD 屏幕上作為主顯示器

```bash
# 激活虛擬顯示
build\bin\Release\vddctl.exe activate --name "VDD Primary" --width 1920 --height 1080 --refresh 90

# 設置為主顯示器
build\bin\Release\vddctl.exe setprimary --index 0

# 檢查狀態
build\bin\Release\vddctl.exe status
```

## 🔧 **測試腳本**

### **自動化測試**

#### **1. 基本功能測試**
```bash
# 運行基本測試
.\simple_vdd_test.bat
```

#### **2. 完整功能測試**
```bash
# 運行完整測試
.\complete_vdd_test.bat
```

#### **3. 管理員權限測試**
```bash
# 以管理員身份運行
.\test_admin_install.bat
```

#### **4. 路徑修復測試**
```bash
# 測試路徑修復
.\test_fixed_path.bat
```

## 🐛 **故障排除**

### **常見問題**

#### **1. 權限問題**
```
錯誤: Access is denied
解決: 以管理員身份運行命令提示符
```

#### **2. 路徑問題**
```
錯誤: The system cannot find the path specified
解決: 確保工作目錄正確，或使用完整路徑
```

#### **3. INF 文件問題**
```
錯誤: Missing or invalid driver package
解決: 使用 IddSampleDriver_Fixed.inf 文件
```

#### **4. SDK 狀態問題**
```
錯誤: SDK not initialized
解決: 先運行 vddctl init
```

#### **5. 驅動安裝問題**
```
錯誤: Failed to install driver
解決: 檢查管理員權限和 INF 文件
```

### **調試步驟**

#### **步驟 1: 檢查環境**
```bash
# 檢查管理員權限
net session

# 檢查驅動是否已安裝
pnputil /enum-drivers | findstr -i "idd"

# 檢查文件存在性
dir build\bin\Release\vddctl.exe
dir IddSampleDriver_Fixed.inf
```

#### **步驟 2: 檢查狀態**
```bash
# 初始化 SDK
build\bin\Release\vddctl.exe init

# 檢查狀態
build\bin\Release\vddctl.exe status

# 檢查版本
build\bin\Release\vddctl.exe version
```

#### **步驟 3: 測試功能**
```bash
# 測試基本功能
build\bin\Release\vddctl.exe help

# 測試驅動安裝
build\bin\Release\vddctl.exe install --path "IddSampleDriver_Fixed.inf"

# 測試虛擬顯示
build\bin\Release\vddctl.exe activate --name "Test" --width 1920 --height 1080 --refresh 60
```

## 📊 **性能監控**

### **系統資源使用**
```bash
# 檢查內存使用
tasklist | findstr vddctl

# 檢查驅動狀態
pnputil /enum-devices /class Display | findstr -i "idd"

# 檢查服務狀態
sc query WUDFRd
```

### **日誌記錄**
```bash
# 啟用詳細日誌
set VDD_DEBUG=1
build\bin\Release\vddctl.exe init

# 檢查事件日誌
eventvwr.msc
# 查看 Windows 日誌 > 系統
```

## 🔒 **安全考慮**

### **權限要求**
- 管理員權限用於驅動安裝
- 普通用戶權限用於日常操作
- 服務帳戶權限用於後台運行

### **安全最佳實踐**
- 定期更新驅動
- 監控系統資源使用
- 限制虛擬顯示數量
- 使用安全的通信協議

## 📈 **性能優化**

### **系統優化**
```bash
# 調整虛擬內存
# 系統屬性 > 高級 > 性能設置 > 高級 > 虛擬內存

# 優化顯示設置
# 顯示設置 > 高級顯示設置 > 顯示適配器屬性
```

### **應用優化**
- 使用適當的顯示分辨率
- 限制刷新率
- 優化內存使用
- 使用硬件加速

## 🚀 **高級用法**

### **批量操作**
```bash
# 批量激活多個顯示器
for /L %i in (1,1,3) do (
    build\bin\Release\vddctl.exe activate --name "VDD %i" --width 1920 --height 1080 --refresh 60
)
```

### **腳本集成**
```bash
# PowerShell 集成
$result = & "build\bin\Release\vddctl.exe" status
Write-Host "Status: $result"

# 批處理集成
call build\bin\Release\vddctl.exe init
if %errorlevel% == 0 (
    echo SDK initialized successfully
) else (
    echo SDK initialization failed
)
```

### **自動化部署**
```bash
# 自動安裝腳本
@echo off
echo Installing VDD SDK...
pnputil /add-driver "IddSampleDriver_Fixed.inf" /install
if %errorlevel% == 0 (
    echo Driver installed successfully
    build\bin\Release\vddctl.exe init
) else (
    echo Driver installation failed
)
```

---

**文檔版本**: 1.0  
**更新日期**: 2025-10-27  
**狀態**: 生產就緒
