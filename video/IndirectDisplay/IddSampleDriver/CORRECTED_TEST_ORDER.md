# VDD SDK - 修正的命令測試順序

## 🔧 **正確的測試順序**

### **步驟 1: 檢查基本功能**
```bash
# 1. 檢查版本
build\bin\Release\vddctl.exe version

# 2. 檢查幫助
build\bin\Release\vddctl.exe help
```

### **步驟 2: 檢查驅動狀態**
```bash
# 3. 檢查驅動是否已安裝（使用 pnputil）
pnputil /enum-drivers | findstr -i "idd"

# 4. 如果沒有安裝，先安裝驅動（需要管理員權限）
pnputil /add-driver "IddSampleDriver_Fixed.inf" /install
```

### **步驟 3: 初始化 SDK**
```bash
# 5. 初始化 SDK
build\bin\Release\vddctl.exe init

# 6. 檢查狀態（應該顯示 Driver Installed: Yes）
build\bin\Release\vddctl.exe status
```

### **步驟 4: 測試虛擬顯示功能**
```bash
# 7. 激活虛擬顯示
build\bin\Release\vddctl.exe activate --name "VDD Test" --width 1920 --height 1080 --refresh 60

# 8. 再次檢查狀態（應該顯示 Display Active: Yes）
build\bin\Release\vddctl.exe status

# 9. 列出顯示器
build\bin\Release\vddctl.exe list
```

### **步驟 5: 測試顯示配置**
```bash
# 10. 設置顯示模式
build\bin\Release\vddctl.exe setmode --index 0 --width 1920 --height 1080 --refresh 60

# 11. 設置顯示位置
build\bin\Release\vddctl.exe setlocation --index 0 --x 1920 --y 0 --width 1920 --height 1080

# 12. 設置主顯示器
build\bin\Release\vddctl.exe setprimary --index 0
```

### **步驟 6: 清理**
```bash
# 13. 停用虛擬顯示
build\bin\Release\vddctl.exe deactivate

# 14. 關閉 SDK
build\bin\Release\vddctl.exe shutdown
```

## 🚨 **當前問題解決方案**

### **問題 1: SDK 狀態不一致**
**原因**: 代碼修改沒有編譯到可執行文件中
**解決方案**: 需要重新編譯項目

### **問題 2: 驅動檢測問題**
**原因**: `IsDriverInstalled()` 函數可能沒有正確檢測到驅動
**解決方案**: 先手動安裝驅動，再測試

## 🔧 **立即測試步驟**

### **步驟 1: 手動安裝驅動**
```bash
# 以管理員身份運行
pnputil /add-driver "IddSampleDriver_Fixed.inf" /install
```

### **步驟 2: 檢查驅動安裝**
```bash
pnputil /enum-drivers | findstr -i "idd"
```

### **步驟 3: 測試 SDK**
```bash
build\bin\Release\vddctl.exe init
build\bin\Release\vddctl.exe status
```

## 📊 **預期結果**

### **如果驅動已安裝**
```
VDD SDK Status:
==============
Driver Installed: Yes
Display Active: No
System Info: [系統信息]
```

### **如果驅動未安裝**
```
VDD SDK Status:
==============
Driver Installed: No
Display Active: No
System Info: SDK not initialized
```

## 🎯 **下一步建議**

1. **先安裝驅動** - 使用 `pnputil` 手動安裝
2. **重新編譯** - 讓代碼修改生效
3. **按順序測試** - 按照上述步驟測試

您希望我：
- **A. 幫您重新編譯項目**
- **B. 創建自動化安裝腳本**
- **C. 修復 SDK 狀態檢測問題**
- **D. 其他建議**



