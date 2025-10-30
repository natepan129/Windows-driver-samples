# VDD SDK - 設計文檔命令完整列表

## 📋 **所有命令用法列表**

### **🔧 基本命令**

#### **1. 版本和幫助**
```bash
# 顯示版本信息
build\bin\Release\vddctl.exe version

# 顯示幫助信息
build\bin\Release\vddctl.exe help
```

#### **2. SDK 管理**
```bash
# 初始化 SDK
build\bin\Release\vddctl.exe init

# 檢查狀態
build\bin\Release\vddctl.exe status

# 關閉 SDK
build\bin\Release\vddctl.exe shutdown
```

#### **3. 驅動管理**
```bash
# 安裝驅動
build\bin\Release\vddctl.exe install --path "IddSampleDriver_Fixed.inf"

# 卸載驅動
build\bin\Release\vddctl.exe uninstall
```

### **🎮 虛擬顯示管理**

#### **4. 激活虛擬顯示**
```bash
# 基本激活
build\bin\Release\vddctl.exe activate --name "VDD Test" --width 1920 --height 1080 --refresh 60

# 高分辨率激活
build\bin\Release\vddctl.exe activate --name "VDD 4K" --width 3840 --height 2160 --refresh 60

# 高刷新率激活
build\bin\Release\vddctl.exe activate --name "VDD Gaming" --width 1920 --height 1080 --refresh 144
```

#### **5. 停用虛擬顯示**
```bash
# 停用虛擬顯示
build\bin\Release\vddctl.exe deactivate
```

#### **6. 列出顯示器**
```bash
# 列出所有顯示器和適配器
build\bin\Release\vddctl.exe list
```

### **⚙️ 顯示配置**

#### **7. 設置顯示模式**
```bash
# 設置 1080p 60Hz
build\bin\Release\vddctl.exe setmode --index 0 --width 1920 --height 1080 --refresh 60

# 設置 1440p 75Hz
build\bin\Release\vddctl.exe setmode --index 0 --width 2560 --height 1440 --refresh 75

# 設置 4K 60Hz
build\bin\Release\vddctl.exe setmode --index 0 --width 3840 --height 2160 --refresh 60
```

#### **8. 設置顯示位置**
```bash
# 設置在第二個屏幕位置
build\bin\Release\vddctl.exe setlocation --index 0 --x 1920 --y 0 --width 1920 --height 1080

# 設置在第三個屏幕位置
build\bin\Release\vddctl.exe setlocation --index 0 --x 3840 --y 0 --width 1920 --height 1080

# 設置重疊位置
build\bin\Release\vddctl.exe setlocation --index 0 --x 100 --y 100 --width 1920 --height 1080
```

#### **9. 設置主顯示器**
```bash
# 設置為主顯示器
build\bin\Release\vddctl.exe setprimary --index 0
```

### **🎯 三種使用場景**

#### **場景 1: VDD 作為顯示代理**
```bash
# 完整流程
build\bin\Release\vddctl.exe init
build\bin\Release\vddctl.exe install --path "IddSampleDriver_Fixed.inf"
build\bin\Release\vddctl.exe activate --name "VDD Proxy" --width 1920 --height 1080 --refresh 120
build\bin\Release\vddctl.exe setlocation --index 0 --x 3840 --y 0 --width 1920 --height 1080
build\bin\Release\vddctl.exe status
```

#### **場景 2: VDD 作為次要顯示器**
```bash
# 次要顯示器流程
build\bin\Release\vddctl.exe activate --name "VDD Remote" --width 2560 --height 1440 --refresh 90
build\bin\Release\vddctl.exe setlocation --index 0 --x 3840 --y 0 --width 2560 --height 1440
build\bin\Release\vddctl.exe status
```

#### **場景 3: VDD 作為主顯示器**
```bash
# 主顯示器流程
build\bin\Release\vddctl.exe activate --name "VDD Primary" --width 1920 --height 1080 --refresh 90
build\bin\Release\vddctl.exe setprimary --index 0
build\bin\Release\vddctl.exe status
```

### **🧪 測試命令序列**

#### **基本功能測試**
```bash
# 1. 檢查版本
build\bin\Release\vddctl.exe version

# 2. 初始化
build\bin\Release\vddctl.exe init

# 3. 檢查狀態
build\bin\Release\vddctl.exe status

# 4. 激活顯示
build\bin\Release\vddctl.exe activate --name "Test" --width 1920 --height 1080 --refresh 60

# 5. 列出顯示器
build\bin\Release\vddctl.exe list

# 6. 設置模式
build\bin\Release\vddctl.exe setmode --index 0 --width 1920 --height 1080 --refresh 60

# 7. 設置位置
build\bin\Release\vddctl.exe setlocation --index 0 --x 1920 --y 0 --width 1920 --height 1080

# 8. 設置主顯示器
build\bin\Release\vddctl.exe setprimary --index 0

# 9. 停用顯示
build\bin\Release\vddctl.exe deactivate

# 10. 關閉 SDK
build\bin\Release\vddctl.exe shutdown
```

#### **驅動管理測試**
```bash
# 1. 檢查驅動狀態
pnputil /enum-drivers | findstr -i "idd"

# 2. 安裝驅動（需要管理員權限）
pnputil /add-driver "IddSampleDriver_Fixed.inf" /install

# 3. 使用 vddctl 安裝
build\bin\Release\vddctl.exe install --path "IddSampleDriver_Fixed.inf"

# 4. 檢查安裝結果
build\bin\Release\vddctl.exe status
```

### **🔍 錯誤處理測試**

#### **權限測試**
```bash
# 非管理員權限測試
build\bin\Release\vddctl.exe install --path "IddSampleDriver_Fixed.inf"
# 應該顯示 "Access is denied" 或類似錯誤
```

#### **參數驗證測試**
```bash
# 無效參數測試
build\bin\Release\vddctl.exe activate --name "Test" --width 0 --height 0 --refresh 0
# 應該顯示參數錯誤

# 缺少參數測試
build\bin\Release\vddctl.exe activate
# 應該顯示缺少參數錯誤
```

### **📊 性能測試**

#### **多顯示器測試**
```bash
# 激活多個虛擬顯示
build\bin\Release\vddctl.exe activate --name "VDD 1" --width 1920 --height 1080 --refresh 60
build\bin\Release\vddctl.exe activate --name "VDD 2" --width 1920 --height 1080 --refresh 60
build\bin\Release\vddctl.exe activate --name "VDD 3" --width 1920 --height 1080 --refresh 60

# 檢查狀態
build\bin\Release\vddctl.exe status
build\bin\Release\vddctl.exe list
```

### **🎯 完整工作流程測試**

#### **端到端測試**
```bash
# 1. 初始化
build\bin\Release\vddctl.exe init

# 2. 安裝驅動
build\bin\Release\vddctl.exe install --path "IddSampleDriver_Fixed.inf"

# 3. 激活虛擬顯示
build\bin\Release\vddctl.exe activate --name "VDD Complete Test" --width 1920 --height 1080 --refresh 60

# 4. 配置顯示
build\bin\Release\vddctl.exe setmode --index 0 --width 1920 --height 1080 --refresh 60
build\bin\Release\vddctl.exe setlocation --index 0 --x 1920 --y 0 --width 1920 --height 1080
build\bin\Release\vddctl.exe setprimary --index 0

# 5. 檢查結果
build\bin\Release\vddctl.exe status
build\bin\Release\vddctl.exe list

# 6. 清理
build\bin\Release\vddctl.exe deactivate
build\bin\Release\vddctl.exe shutdown
```

## 📝 **測試檢查清單**

### **基本功能檢查**
- [ ] `vddctl version` - 顯示版本信息
- [ ] `vddctl help` - 顯示幫助信息
- [ ] `vddctl init` - 初始化 SDK
- [ ] `vddctl status` - 檢查狀態
- [ ] `vddctl shutdown` - 關閉 SDK

### **驅動管理檢查**
- [ ] `vddctl install` - 安裝驅動
- [ ] `vddctl uninstall` - 卸載驅動
- [ ] `pnputil /enum-drivers` - 檢查驅動安裝

### **虛擬顯示檢查**
- [ ] `vddctl activate` - 激活虛擬顯示
- [ ] `vddctl deactivate` - 停用虛擬顯示
- [ ] `vddctl list` - 列出顯示器

### **顯示配置檢查**
- [ ] `vddctl setmode` - 設置顯示模式
- [ ] `vddctl setlocation` - 設置顯示位置
- [ ] `vddctl setprimary` - 設置主顯示器

### **場景測試檢查**
- [ ] 場景 1: VDD 作為顯示代理
- [ ] 場景 2: VDD 作為次要顯示器
- [ ] 場景 3: VDD 作為主顯示器

### **錯誤處理檢查**
- [ ] 權限錯誤處理
- [ ] 參數驗證錯誤
- [ ] 缺少參數錯誤

---

**使用說明**:
1. 複製上述命令到命令提示符中執行
2. 確保以管理員身份運行需要權限的命令
3. 檢查每個命令的輸出結果
4. 在檢查清單中標記完成的項目

**注意**: 所有命令都需要在項目目錄中執行，確保 `build\bin\Release\vddctl.exe` 路徑正確。



