# VDD Control Tool - Install Commands

## 🚀 使用 vddctl.exe 安装 IddSampleDriver

---

## 基本安装命令

### 命令格式
```batch
vddctl install --inf <INF文件路径>
```

### 最简单的安装
```batch
vddctl install --inf IddSampleDriver_Fixed.inf
```

---

## ✅ 完整安装流程

### 方法 1: 手动逐步执行

```batch
REM Step 1: 初始化 SDK
vddctl init

REM Step 2: 安装驱动（需要管理员权限，会弹出 UAC）
vddctl install --inf IddSampleDriver_Fixed.inf

REM Step 3: 验证安装
vddctl status
```

### 方法 2: 使用脚本（推荐）

```batch
cmd /c install_with_vddctl.bat
```

### 方法 3: 使用管理员权限运行

```batch
REM 在 PowerShell 中以管理员身份运行
powershell -Command "Start-Process -FilePath 'build\bin\Release\vddctl.exe' -ArgumentList 'install --inf IddSampleDriver_Fixed.inf' -Verb RunAs -Wait"
```

---

## 📋 详细步骤

### 1. 准备文件

确保以下文件存在：
- ✅ `build\bin\Release\vddctl.exe` (已编译的工具)
- ✅ `IddSampleDriver_Fixed.inf` (驱动 INF 文件)
- ✅ `x64\Release\IddSampleDriver.dll` (驱动 DLL)

### 2. 初始化 SDK

```batch
C:\Path\To\Project> vddctl init
```

**输出**:
```
Initializing VDD SDK...
VDD SDK initialized successfully.
```

### 3. 安装驱动

```batch
C:\Path\To\Project> vddctl install --inf IddSampleDriver_Fixed.inf
```

**输出**:
```
Installing driver...
Driver installed successfully.
```

⚠️ **注意**: 
- 会弹出 UAC 提示，点击 "是" 允许
- 如果已有旧设备，会自动更新

### 4. 验证安装

```batch
C:\Path\To\Project> vddctl status
```

**成功的输出**:
```
VDD SDK Status:
==============
Driver Installed: Yes
Driver Version: 1.0.0
Display Active: No
System Info: Windows 10/11 x64
```

---

## 🔍 验证安装成功

### 使用 vddctl 检查
```batch
vddctl status
```
应该显示: `Driver Installed: Yes`

### 使用 PowerShell 检查
```powershell
Get-PnpDevice | Where-Object { $_.FriendlyName -like '*IddSampleDriver*' }
```

**成功的输出**:
```
FriendlyName           Status Class   ClassGuid
------------           ------ -----   ---------
IddSampleDriver Device OK     Display {4d36e968-e325-11ce-bfc1-08002be10318}
```

### 在 Device Manager 中检查
1. 按 `Win + X` 打开设备管理器
2. 展开 "Display adapters" (显示适配器)
3. 应该看到 "IddSampleDriver Device"

---

## 🛠️ 完整示例脚本

### simple_install.bat
```batch
@echo off
echo Installing IddSampleDriver with vddctl...

REM Initialize
vddctl init

REM Install (will show UAC prompt)
vddctl install --inf IddSampleDriver_Fixed.inf

REM Verify
vddctl status

echo.
echo Installation complete!
pause
```

### install_with_admin.bat
```batch
@echo off
echo Installing IddSampleDriver with admin rights...

set VDDCTL=%CD%\build\bin\Release\vddctl.exe
set INF_PATH=%CD%\IddSampleDriver_Fixed.inf

REM Initialize
"%VDDCTL%" init

REM Install with admin rights
powershell -Command "Start-Process '%VDDCTL%' -ArgumentList 'install --inf \"%INF_PATH%\"' -Verb RunAs -Wait"

REM Verify
"%VDDCTL%" status

pause
```

---

## ❌ 常见错误和解决方案

### 错误 1: "vddctl.exe not found"

**原因**: vddctl 未编译

**解决方案**:
```batch
cmd /c compile_sdk_only.bat
```

### 错误 2: "INF file not found"

**原因**: INF 路径不正确

**解决方案**:
```batch
REM 使用绝对路径
vddctl install --inf "C:\Full\Path\To\IddSampleDriver_Fixed.inf"
```

### 错误 3: "Access Denied" (错误 5)

**原因**: 没有管理员权限

**解决方案**:
```batch
REM 右键点击 cmd/PowerShell，选择 "以管理员身份运行"
REM 或使用 PowerShell 提升权限：
powershell -Command "Start-Process 'vddctl.exe' -ArgumentList 'install --inf IddSampleDriver_Fixed.inf' -Verb RunAs -Wait"
```

### 错误 4: "Driver Installed: No" (安装后)

**原因**: 安装可能失败

**解决方案**:
```batch
REM 1. 检查 INF 文件编码（应该是 UTF-16 LE 或 ANSI）
REM 2. 检查错误日志
vddctl install --inf IddSampleDriver_Fixed.inf

REM 3. 验证文件
dir IddSampleDriver_Fixed.inf
dir x64\Release\IddSampleDriver.dll
```

### 错误 5: Device 没有 Display Class

**原因**: 
- INF 文件错误
- 使用了错误的 INF（UTF-8 vs UTF-16）

**解决方案**:
```batch
REM 使用 IddSampleDriver_Fixed.inf (已验证可用)
vddctl uninstall
vddctl install --inf IddSampleDriver_Fixed.inf
```

---

## 📊 命令对比

| 操作 | install_driver.exe | vddctl.exe |
|------|-------------------|------------|
| 安装命令 | `install_driver.exe IddSampleDriver_Fixed.inf` | `vddctl install --inf IddSampleDriver_Fixed.inf` |
| 需要管理员 | ✅ Yes | ✅ Yes |
| 支持查看状态 | ❌ No | ✅ Yes (`vddctl status`) |
| 支持卸载 | ❌ No (需要 uninstall_driver.exe) | ✅ Yes (`vddctl uninstall`) |
| 支持激活显示 | ❌ No | ✅ Yes (`vddctl activate`) |
| 设计文档推荐 | ❌ No | ✅ Yes |

**推荐使用**: `vddctl.exe` (功能更完整，是设计文档的正式工具)

---

## 🎯 快速参考

### 安装
```batch
vddctl install --inf IddSampleDriver_Fixed.inf
```

### 卸载
```batch
vddctl uninstall
```

### 检查状态
```batch
vddctl status
```

### 激活显示器
```batch
vddctl activate --width 1920 --height 1080
```

### 停用显示器
```batch
vddctl deactivate
```

---

## 📝 完整工作流程

```batch
@echo off
REM ========================================
REM Complete IddSampleDriver Installation
REM ========================================

REM 1. Initialize SDK
echo [1/6] Initializing SDK...
vddctl init

REM 2. Check current status
echo [2/6] Checking status...
vddctl status

REM 3. Install driver
echo [3/6] Installing driver...
vddctl install --inf IddSampleDriver_Fixed.inf

REM 4. Verify installation
echo [4/6] Verifying installation...
vddctl status

REM 5. Activate virtual display
echo [5/6] Activating virtual display...
vddctl activate --width 1920 --height 1080

REM 6. List all displays
echo [6/6] Listing displays...
vddctl list

echo.
echo Installation and activation complete!
pause
```

---

## ⚡ 一键安装脚本

创建文件 `quick_install_vddctl.bat`:

```batch
@echo off
set VDDCTL=build\bin\Release\vddctl.exe
set INF=IddSampleDriver_Fixed.inf

echo Installing IddSampleDriver...
"%VDDCTL%" init
powershell -Command "Start-Process '%VDDCTL%' -ArgumentList 'install --inf %INF%' -Verb RunAs -Wait"
"%VDDCTL%" status
echo Done!
pause
```

运行:
```batch
cmd /c quick_install_vddctl.bat
```

---

## 🔗 相关文档

- **完整命令参考**: `VDDCTL_COMMAND_REFERENCE.md`
- **基本功能验证**: `VDDSDK_BASIC_VERIFICATION.md`
- **测试脚本**: `install_with_vddctl.bat`

