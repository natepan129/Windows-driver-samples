# VDD Control Tool - 完整命令参考

## ✅ 安装成功后的测试指令

---

## 📋 基础命令

### 1. 查看状态
```batch
vddctl.exe status
```

**预期输出：**
```
VDD SDK Status:
==============
Driver Installed: Yes
Display Active: No
System Info: Windows 6.2 Build 9200
Total Memory: 16047 MB
Available Memory: 7109 MB
Processors: 8
Architecture: x64
```

---

### 2. 列出显示适配器
```batch
vddctl.exe list
```

**预期输出：**
```
Available Display Adapters:
===========================
Adapter 0: VirtualBox Graphics Adapter (WDDM)
Adapter 1: IddSampleDriver (if activated)
```

---

### 3. 枚举支持的显示模式
```batch
vddctl.exe enum-modes
```

**预期输出：**
```
Supported Display Modes:
========================
1920x1080 @ 60Hz
1920x1080 @ 30Hz
1600x1200 @ 60Hz
1280x720 @ 60Hz
...
```

---

## 🚀 高级命令

### 4. 激活虚拟显示器
```batch
vddctl.exe activate
```

**说明：**
- 创建一个虚拟显示器
- 显示器会出现在"显示设置"中
- 可以设置为主显示器或扩展显示器

**预期输出：**
```
Activating virtual display...
Virtual display activated successfully.
Display ID: 1
```

---

### 5. 停用虚拟显示器
```batch
vddctl.exe deactivate
```

**预期输出：**
```
Deactivating virtual display...
Virtual display deactivated successfully.
```

---

### 6. 设置显示模式
```batch
vddctl.exe setmode --width 1920 --height 1080 --refresh 60
```

**参数：**
- `--width`: 宽度（像素）
- `--height`: 高度（像素）
- `--refresh`: 刷新率（Hz，可选，默认 60）

**常用分辨率：**
```batch
# Full HD
vddctl.exe setmode --width 1920 --height 1080

# HD
vddctl.exe setmode --width 1280 --height 720

# 4K
vddctl.exe setmode --width 3840 --height 2160

# Custom
vddctl.exe setmode --width 2560 --height 1440 --refresh 144
```

---

### 7. 设置显示器位置
```batch
vddctl.exe setlocation --x 1920 --y 0
```

**参数：**
- `--x`: X 坐标（像素）
- `--y`: Y 坐标（像素）

**使用场景：**
- 双显示器扩展桌面时调整相对位置
- 左右排列：`--x 1920 --y 0`（假设主屏幕 1920x1080）
- 上下排列：`--x 0 --y 1080`

---

### 8. 设置为主显示器
```batch
vddctl.exe setprimary
```

**警告：**
⚠️ 将虚拟显示器设置为主显示器可能导致物理屏幕黑屏！
⚠️ 仅在远程桌面或远程控制场景下使用！

**预期输出：**
```
Setting virtual display as primary...
Primary display changed successfully.
```

---

### 9. 卸载驱动
```batch
vddctl.exe uninstall
```

**说明：**
- 停用所有虚拟显示器
- 移除驱动程序
- 清理设备管理器中的设备

**预期输出：**
```
Uninstalling driver...
Deactivating displays...
Removing device...
Cleaning driver store...
Driver uninstalled successfully.
```

---

## 📝 完整测试流程

### 方案 A: 自动测试脚本
```batch
# 以管理员身份运行
test_vddctl_commands.bat
```

### 方案 B: 手动测试（推荐）

#### 步骤 1: 验证安装
```batch
vddctl.exe status
```
确认 `Driver Installed: Yes`

#### 步骤 2: 查看适配器
```batch
vddctl.exe list
```

#### 步骤 3: 激活虚拟显示器
```batch
vddctl.exe activate
```

#### 步骤 4: 验证激活
```batch
vddctl.exe status
```
确认 `Display Active: Yes`

#### 步骤 5: 设置分辨率
```batch
vddctl.exe setmode --width 1920 --height 1080
```

#### 步骤 6: 测试位置调整
```batch
vddctl.exe setlocation --x 1920 --y 0
```

#### 步骤 7: 在 Windows 显示设置中验证
```batch
# 打开显示设置
ms-settings:display
```
应该看到两个显示器

#### 步骤 8: 停用虚拟显示器
```batch
vddctl.exe deactivate
```

#### 步骤 9: 最终验证
```batch
vddctl.exe status
```
确认 `Display Active: No`

---

## 🔍 故障排除

### 问题：`vddctl.exe` 命令无法识别

**解决方案：**
```batch
# 使用完整路径
"C:\Path\To\IddSampleDriver\vddctl.exe" status

# 或者切换到目录
cd "C:\Path\To\IddSampleDriver"
.\vddctl.exe status
```

---

### 问题：激活失败

**检查：**
```batch
# 1. 确认驱动已安装
vddctl.exe status

# 2. 以管理员身份运行
# 右键 PowerShell/CMD -> 以管理员身份运行
vddctl.exe activate

# 3. 查看设备管理器
devmgmt.msc
# 展开 "显示适配器"
# 查找 "IddSampleDriver"
```

---

### 问题：设置模式失败

**可能原因：**
1. 显示器未激活 → 先运行 `vddctl.exe activate`
2. 分辨率不支持 → 先运行 `vddctl.exe enum-modes` 查看支持的模式
3. 权限不足 → 以管理员身份运行

---

## 📊 验证成功的标志

### ✅ 安装成功
- [ ] `vddctl.exe status` 显示 `Driver Installed: Yes`
- [ ] 设备管理器中看到 "IddSampleDriver"
- [ ] 无黄色感叹号或红色叉号

### ✅ 激活成功
- [ ] `vddctl.exe status` 显示 `Display Active: Yes`
- [ ] Windows 显示设置中出现第二个显示器
- [ ] 可以拖动窗口到虚拟显示器

### ✅ 功能正常
- [ ] 可以设置分辨率
- [ ] 可以调整显示器位置
- [ ] 可以在虚拟显示器上全屏应用
- [ ] 可以停用虚拟显示器
- [ ] 可以重新激活虚拟显示器

---

## 🎯 使用场景示例

### 场景 1: 远程桌面扩展屏幕
```batch
# 激活虚拟显示器
vddctl.exe activate

# 设置为 Full HD
vddctl.exe setmode --width 1920 --height 1080

# 放在主屏幕右侧
vddctl.exe setlocation --x 1920 --y 0
```

### 场景 2: 无头服务器显示输出
```batch
# 安装驱动
vddctl.exe install x64\Release\IddSampleDriver\IddSampleDriver.inf

# 激活虚拟显示器
vddctl.exe activate

# 设置为主显示器（慎用！）
vddctl.exe setprimary
```

### 场景 3: 开发测试多屏应用
```batch
# 创建虚拟显示器
vddctl.exe activate

# 设置不同分辨率测试
vddctl.exe setmode --width 1280 --height 720
# 测试应用...

vddctl.exe setmode --width 1920 --height 1080
# 再次测试...

# 完成后清理
vddctl.exe deactivate
```

---

## 📚 相关文件

- `test_install.bat` - 安装驱动测试脚本
- `test_vddctl_commands.bat` - 命令测试脚本
- `INSTALL_TEST_GUIDE.md` - 安装指南
- `VDD_SDK_COMMANDS.md` - SDK API 文档

---

## 🔗 PowerShell 一键测试

```powershell
# 创建测试脚本
@"
Write-Host "=== VDD Driver Test ===" -ForegroundColor Cyan
Write-Host ""

Write-Host "[1] Status Check" -ForegroundColor Yellow
.\vddctl.exe status
Write-Host ""

Write-Host "[2] List Adapters" -ForegroundColor Yellow
.\vddctl.exe list
Write-Host ""

Write-Host "[3] Activate Display" -ForegroundColor Yellow
.\vddctl.exe activate
Write-Host ""

Write-Host "[4] Status After Activate" -ForegroundColor Yellow
.\vddctl.exe status
Write-Host ""

Write-Host "[5] Deactivate Display" -ForegroundColor Yellow
.\vddctl.exe deactivate
Write-Host ""

Write-Host "[6] Final Status" -ForegroundColor Yellow
.\vddctl.exe status
Write-Host ""

Write-Host "=== Test Complete ===" -ForegroundColor Green
"@ | Out-File -FilePath test_quick.ps1 -Encoding UTF8

# 运行测试
powershell -ExecutionPolicy Bypass -File test_quick.ps1
```

---

**当前状态：** ✅ 驱动已成功安装  
**下一步：** 运行上述测试命令验证功能  
**版本：** 2025-11-03

# VDD Control Tool - 完整命令参考

## ✅ 安装成功后的测试指令

---

## 📋 基础命令

### 1. 查看状态
```batch
vddctl.exe status
```

**预期输出：**
```
VDD SDK Status:
==============
Driver Installed: Yes
Display Active: No
System Info: Windows 6.2 Build 9200
Total Memory: 16047 MB
Available Memory: 7109 MB
Processors: 8
Architecture: x64
```

---

### 2. 列出显示适配器
```batch
vddctl.exe list
```

**预期输出：**
```
Available Display Adapters:
===========================
Adapter 0: VirtualBox Graphics Adapter (WDDM)
Adapter 1: IddSampleDriver (if activated)
```

---

### 3. 枚举支持的显示模式
```batch
vddctl.exe enum-modes
```

**预期输出：**
```
Supported Display Modes:
========================
1920x1080 @ 60Hz
1920x1080 @ 30Hz
1600x1200 @ 60Hz
1280x720 @ 60Hz
...
```

---

## 🚀 高级命令

### 4. 激活虚拟显示器
```batch
vddctl.exe activate
```

**说明：**
- 创建一个虚拟显示器
- 显示器会出现在"显示设置"中
- 可以设置为主显示器或扩展显示器

**预期输出：**
```
Activating virtual display...
Virtual display activated successfully.
Display ID: 1
```

---

### 5. 停用虚拟显示器
```batch
vddctl.exe deactivate
```

**预期输出：**
```
Deactivating virtual display...
Virtual display deactivated successfully.
```

---

### 6. 设置显示模式
```batch
vddctl.exe setmode --width 1920 --height 1080 --refresh 60
```

**参数：**
- `--width`: 宽度（像素）
- `--height`: 高度（像素）
- `--refresh`: 刷新率（Hz，可选，默认 60）

**常用分辨率：**
```batch
# Full HD
vddctl.exe setmode --width 1920 --height 1080

# HD
vddctl.exe setmode --width 1280 --height 720

# 4K
vddctl.exe setmode --width 3840 --height 2160

# Custom
vddctl.exe setmode --width 2560 --height 1440 --refresh 144
```

---

### 7. 设置显示器位置
```batch
vddctl.exe setlocation --x 1920 --y 0
```

**参数：**
- `--x`: X 坐标（像素）
- `--y`: Y 坐标（像素）

**使用场景：**
- 双显示器扩展桌面时调整相对位置
- 左右排列：`--x 1920 --y 0`（假设主屏幕 1920x1080）
- 上下排列：`--x 0 --y 1080`

---

### 8. 设置为主显示器
```batch
vddctl.exe setprimary
```

**警告：**
⚠️ 将虚拟显示器设置为主显示器可能导致物理屏幕黑屏！
⚠️ 仅在远程桌面或远程控制场景下使用！

**预期输出：**
```
Setting virtual display as primary...
Primary display changed successfully.
```

---

### 9. 卸载驱动
```batch
vddctl.exe uninstall
```

**说明：**
- 停用所有虚拟显示器
- 移除驱动程序
- 清理设备管理器中的设备

**预期输出：**
```
Uninstalling driver...
Deactivating displays...
Removing device...
Cleaning driver store...
Driver uninstalled successfully.
```

---

## 📝 完整测试流程

### 方案 A: 自动测试脚本
```batch
# 以管理员身份运行
test_vddctl_commands.bat
```

### 方案 B: 手动测试（推荐）

#### 步骤 1: 验证安装
```batch
vddctl.exe status
```
确认 `Driver Installed: Yes`

#### 步骤 2: 查看适配器
```batch
vddctl.exe list
```

#### 步骤 3: 激活虚拟显示器
```batch
vddctl.exe activate
```

#### 步骤 4: 验证激活
```batch
vddctl.exe status
```
确认 `Display Active: Yes`

#### 步骤 5: 设置分辨率
```batch
vddctl.exe setmode --width 1920 --height 1080
```

#### 步骤 6: 测试位置调整
```batch
vddctl.exe setlocation --x 1920 --y 0
```

#### 步骤 7: 在 Windows 显示设置中验证
```batch
# 打开显示设置
ms-settings:display
```
应该看到两个显示器

#### 步骤 8: 停用虚拟显示器
```batch
vddctl.exe deactivate
```

#### 步骤 9: 最终验证
```batch
vddctl.exe status
```
确认 `Display Active: No`

---

## 🔍 故障排除

### 问题：`vddctl.exe` 命令无法识别

**解决方案：**
```batch
# 使用完整路径
"C:\Path\To\IddSampleDriver\vddctl.exe" status

# 或者切换到目录
cd "C:\Path\To\IddSampleDriver"
.\vddctl.exe status
```

---

### 问题：激活失败

**检查：**
```batch
# 1. 确认驱动已安装
vddctl.exe status

# 2. 以管理员身份运行
# 右键 PowerShell/CMD -> 以管理员身份运行
vddctl.exe activate

# 3. 查看设备管理器
devmgmt.msc
# 展开 "显示适配器"
# 查找 "IddSampleDriver"
```

---

### 问题：设置模式失败

**可能原因：**
1. 显示器未激活 → 先运行 `vddctl.exe activate`
2. 分辨率不支持 → 先运行 `vddctl.exe enum-modes` 查看支持的模式
3. 权限不足 → 以管理员身份运行

---

## 📊 验证成功的标志

### ✅ 安装成功
- [ ] `vddctl.exe status` 显示 `Driver Installed: Yes`
- [ ] 设备管理器中看到 "IddSampleDriver"
- [ ] 无黄色感叹号或红色叉号

### ✅ 激活成功
- [ ] `vddctl.exe status` 显示 `Display Active: Yes`
- [ ] Windows 显示设置中出现第二个显示器
- [ ] 可以拖动窗口到虚拟显示器

### ✅ 功能正常
- [ ] 可以设置分辨率
- [ ] 可以调整显示器位置
- [ ] 可以在虚拟显示器上全屏应用
- [ ] 可以停用虚拟显示器
- [ ] 可以重新激活虚拟显示器

---

## 🎯 使用场景示例

### 场景 1: 远程桌面扩展屏幕
```batch
# 激活虚拟显示器
vddctl.exe activate

# 设置为 Full HD
vddctl.exe setmode --width 1920 --height 1080

# 放在主屏幕右侧
vddctl.exe setlocation --x 1920 --y 0
```

### 场景 2: 无头服务器显示输出
```batch
# 安装驱动
vddctl.exe install x64\Release\IddSampleDriver\IddSampleDriver.inf

# 激活虚拟显示器
vddctl.exe activate

# 设置为主显示器（慎用！）
vddctl.exe setprimary
```

### 场景 3: 开发测试多屏应用
```batch
# 创建虚拟显示器
vddctl.exe activate

# 设置不同分辨率测试
vddctl.exe setmode --width 1280 --height 720
# 测试应用...

vddctl.exe setmode --width 1920 --height 1080
# 再次测试...

# 完成后清理
vddctl.exe deactivate
```

---

## 📚 相关文件

- `test_install.bat` - 安装驱动测试脚本
- `test_vddctl_commands.bat` - 命令测试脚本
- `INSTALL_TEST_GUIDE.md` - 安装指南
- `VDD_SDK_COMMANDS.md` - SDK API 文档

---

## 🔗 PowerShell 一键测试

```powershell
# 创建测试脚本
@"
Write-Host "=== VDD Driver Test ===" -ForegroundColor Cyan
Write-Host ""

Write-Host "[1] Status Check" -ForegroundColor Yellow
.\vddctl.exe status
Write-Host ""

Write-Host "[2] List Adapters" -ForegroundColor Yellow
.\vddctl.exe list
Write-Host ""

Write-Host "[3] Activate Display" -ForegroundColor Yellow
.\vddctl.exe activate
Write-Host ""

Write-Host "[4] Status After Activate" -ForegroundColor Yellow
.\vddctl.exe status
Write-Host ""

Write-Host "[5] Deactivate Display" -ForegroundColor Yellow
.\vddctl.exe deactivate
Write-Host ""

Write-Host "[6] Final Status" -ForegroundColor Yellow
.\vddctl.exe status
Write-Host ""

Write-Host "=== Test Complete ===" -ForegroundColor Green
"@ | Out-File -FilePath test_quick.ps1 -Encoding UTF8

# 运行测试
powershell -ExecutionPolicy Bypass -File test_quick.ps1
```

---

**当前状态：** ✅ 驱动已成功安装  
**下一步：** 运行上述测试命令验证功能  
**版本：** 2025-11-03

