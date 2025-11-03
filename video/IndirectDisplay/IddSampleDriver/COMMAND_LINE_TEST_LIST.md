# VDD SDK - Design Document Complete Command List

## 📋 **All Command Usage List**

### **🔧 Basic Commands**

#### **1. Version and Help**
```bash
# Display version information
build\bin\Release\vddctl.exe version

# Display help information
build\bin\Release\vddctl.exe help
```

#### **2. SDK Management**
```bash
# Initialize SDK
build\bin\Release\vddctl.exe init

# Check status
build\bin\Release\vddctl.exe status

# Shutdown SDK
build\bin\Release\vddctl.exe shutdown
```

#### **3. Driver Management**
```bash
# Install driver
build\bin\Release\vddctl.exe install --path "IddSampleDriver_Fixed.inf"

# Uninstall driver
build\bin\Release\vddctl.exe uninstall
```

### **🎮 Virtual Display Management**

#### **4. Activate Virtual Display**
```bash
# Basic activation
build\bin\Release\vddctl.exe activate --name "VDD Test" --width 1920 --height 1080 --refresh 60

# High resolution activation
build\bin\Release\vddctl.exe activate --name "VDD 4K" --width 3840 --height 2160 --refresh 60

# High refresh rate activation
build\bin\Release\vddctl.exe activate --name "VDD Gaming" --width 1920 --height 1080 --refresh 144
```

#### **5. Deactivate Virtual Display**
```bash
# Deactivate virtual display
build\bin\Release\vddctl.exe deactivate
```

#### **6. List Displays**
```bash
# List all displays and adapters
build\bin\Release\vddctl.exe list
```

### **⚙️ Display Configuration**

#### **7. Set Display Mode**
```bash
# Set 1080p 60Hz
build\bin\Release\vddctl.exe setmode --index 0 --width 1920 --height 1080 --refresh 60

# Set 1440p 75Hz
build\bin\Release\vddctl.exe setmode --index 0 --width 2560 --height 1440 --refresh 75

# Set 4K 60Hz
build\bin\Release\vddctl.exe setmode --index 0 --width 3840 --height 2160 --refresh 60
```

#### **8. Set Display Location**
```bash
# Set to second screen position
build\bin\Release\vddctl.exe setlocation --index 0 --x 1920 --y 0 --width 1920 --height 1080

# Set to third screen position
build\bin\Release\vddctl.exe setlocation --index 0 --x 3840 --y 0 --width 1920 --height 1080

# Set overlapping position
build\bin\Release\vddctl.exe setlocation --index 0 --x 100 --y 100 --width 1920 --height 1080
```

#### **9. Set Primary Display**
```bash
# Set as primary display
build\bin\Release\vddctl.exe setprimary --index 0
```

### **🎯 Three Usage Scenarios**

#### **Scenario 1: VDD as Display Proxy**
```bash
# Complete workflow
build\bin\Release\vddctl.exe init
build\bin\Release\vddctl.exe install --path "IddSampleDriver_Fixed.inf"
build\bin\Release\vddctl.exe activate --name "VDD Proxy" --width 1920 --height 1080 --refresh 120
build\bin\Release\vddctl.exe setlocation --index 0 --x 3840 --y 0 --width 1920 --height 1080
build\bin\Release\vddctl.exe status
```

#### **Scenario 2: VDD as Secondary Display**
```bash
# Secondary display workflow
build\bin\Release\vddctl.exe activate --name "VDD Remote" --width 2560 --height 1440 --refresh 90
build\bin\Release\vddctl.exe setlocation --index 0 --x 3840 --y 0 --width 2560 --height 1440
build\bin\Release\vddctl.exe status
```

#### **Scenario 3: VDD as Primary Display**
```bash
# Primary display workflow
build\bin\Release\vddctl.exe activate --name "VDD Primary" --width 1920 --height 1080 --refresh 90
build\bin\Release\vddctl.exe setprimary --index 0
build\bin\Release\vddctl.exe status
```

### **🧪 Test Command Sequences**

#### **Basic Function Test**
```bash
# 1. Check version
build\bin\Release\vddctl.exe version

# 2. Initialize
build\bin\Release\vddctl.exe init

# 3. Check status
build\bin\Release\vddctl.exe status

# 4. Activate display
build\bin\Release\vddctl.exe activate --name "Test" --width 1920 --height 1080 --refresh 60

# 5. List displays
build\bin\Release\vddctl.exe list

# 6. Set mode
build\bin\Release\vddctl.exe setmode --index 0 --width 1920 --height 1080 --refresh 60

# 7. Set location
build\bin\Release\vddctl.exe setlocation --index 0 --x 1920 --y 0 --width 1920 --height 1080

# 8. Set primary display
build\bin\Release\vddctl.exe setprimary --index 0

# 9. Deactivate display
build\bin\Release\vddctl.exe deactivate

# 10. Shutdown SDK
build\bin\Release\vddctl.exe shutdown
```

#### **Driver Management Test**
```bash
# 1. Check driver status
pnputil /enum-drivers | findstr -i "idd"

# 2. Install driver (requires administrator privileges)
pnputil /add-driver "IddSampleDriver_Fixed.inf" /install

# 3. Install using vddctl
build\bin\Release\vddctl.exe install --path "IddSampleDriver_Fixed.inf"

# 4. Check installation result
build\bin\Release\vddctl.exe status
```

### **🔍 Error Handling Tests**

#### **Permission Test**
```bash
# Non-administrator privilege test
build\bin\Release\vddctl.exe install --path "IddSampleDriver_Fixed.inf"
# Should display "Access is denied" or similar error
```

#### **Parameter Validation Test**
```bash
# Invalid parameter test
build\bin\Release\vddctl.exe activate --name "Test" --width 0 --height 0 --refresh 0
# Should display parameter error

# Missing parameter test
build\bin\Release\vddctl.exe activate
# Should display missing parameter error
```

### **📊 Performance Tests**

#### **Multi-Display Test**
```bash
# Activate multiple virtual displays
build\bin\Release\vddctl.exe activate --name "VDD 1" --width 1920 --height 1080 --refresh 60
build\bin\Release\vddctl.exe activate --name "VDD 2" --width 1920 --height 1080 --refresh 60
build\bin\Release\vddctl.exe activate --name "VDD 3" --width 1920 --height 1080 --refresh 60

# Check status
build\bin\Release\vddctl.exe status
build\bin\Release\vddctl.exe list
```

### **🎯 Complete Workflow Test**

#### **End-to-End Test**
```bash
# 1. Initialize
build\bin\Release\vddctl.exe init

# 2. Install driver
build\bin\Release\vddctl.exe install --path "IddSampleDriver_Fixed.inf"

# 3. Activate virtual display
build\bin\Release\vddctl.exe activate --name "VDD Complete Test" --width 1920 --height 1080 --refresh 60

# 4. Configure display
build\bin\Release\vddctl.exe setmode --index 0 --width 1920 --height 1080 --refresh 60
build\bin\Release\vddctl.exe setlocation --index 0 --x 1920 --y 0 --width 1920 --height 1080
build\bin\Release\vddctl.exe setprimary --index 0

# 5. Check results
build\bin\Release\vddctl.exe status
build\bin\Release\vddctl.exe list

# 6. Cleanup
build\bin\Release\vddctl.exe deactivate
build\bin\Release\vddctl.exe shutdown
```

## 📝 **Test Checklist**

### **Basic Function Checks**
- [ ] `vddctl version` - Display version information
- [ ] `vddctl help` - Display help information
- [ ] `vddctl init` - Initialize SDK
- [ ] `vddctl status` - Check status
- [ ] `vddctl shutdown` - Shutdown SDK

### **Driver Management Checks**
- [ ] `vddctl install` - Install driver
- [ ] `vddctl uninstall` - Uninstall driver
- [ ] `pnputil /enum-drivers` - Check driver installation

### **Virtual Display Checks**
- [ ] `vddctl activate` - Activate virtual display
- [ ] `vddctl deactivate` - Deactivate virtual display
- [ ] `vddctl list` - List displays

### **Display Configuration Checks**
- [ ] `vddctl setmode` - Set display mode
- [ ] `vddctl setlocation` - Set display location
- [ ] `vddctl setprimary` - Set primary display

### **Scenario Test Checks**
- [ ] Scenario 1: VDD as display proxy
- [ ] Scenario 2: VDD as secondary display
- [ ] Scenario 3: VDD as primary display

### **Error Handling Checks**
- [ ] Permission error handling
- [ ] Parameter validation error
- [ ] Missing parameter error

---

**Usage Instructions**:
1. Copy the above commands into command prompt to execute
2. Ensure commands requiring privileges are run as administrator
3. Check output results for each command
4. Mark completed items in the checklist

**Note**: All commands need to be executed in the project directory, ensure `build\bin\Release\vddctl.exe` path is correct.

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



