# VDD Control Tool (vddctl) - 命令参考

## 📋 概述

`vddctl` 是 VDD SDK 的命令行工具，用于管理虚拟显示驱动。

**位置**: `build\bin\Release\vddctl.exe`

---

## 🚀 基本命令

### 1. `init` - 初始化 SDK

初始化 VDD SDK，准备驱动管理环境。

#### 语法
```bash
vddctl init [--verbose]
```

#### 选项
- `--verbose`: 显示详细日志输出

#### 示例
```batch
REM 基本初始化
vddctl init

REM 详细日志初始化
vddctl init --verbose
```

#### 输出
```
Initializing VDD SDK...
VDD SDK initialized successfully.
```

#### 返回值
- `0`: 成功
- `非0`: 失败（检查错误信息）

---

### 2. `status` - 查看状态

显示 VDD SDK 和驱动的当前状态。

#### 语法
```bash
vddctl status
```

#### 示例
```batch
vddctl status
```

#### 输出
```
VDD SDK Status:
==============
Driver Installed: Yes/No
Display Active: Yes/No
System Info: [系统信息]
```

如果驱动已安装，还会显示：
```
Driver Version: 1.0.0
Active Displays: 2
```

---

### 3. `version` - 查看版本

显示 VDD SDK 的版本信息。

#### 语法
```bash
vddctl version
```

#### 示例
```batch
vddctl version
```

#### 输出
```
VDD SDK Version: 1.0.0
Build Date: Oct 27 2025 14:24:13
```

---

### 4. `install` - 安装驱动

安装虚拟显示驱动到系统。

#### 语法
```bash
vddctl install --inf <INF文件路径>
```

#### 选项
- `--inf <path>`: INF 文件的完整路径（必需）

#### 示例
```batch
REM 使用相对路径
vddctl install --inf IddSampleDriver_Fixed.inf

REM 使用绝对路径
vddctl install --inf "C:\Path\To\IddSampleDriver_Fixed.inf"
```

#### 输出
```
Installing driver...
Driver installed successfully.
```

#### 注意事项
- ⚠️ **需要管理员权限**（会弹出 UAC 提示）
- 使用 `IddSampleDriver_Fixed.inf`（UTF-8 编码）
- DLL 来自 `x64\Release\IddSampleDriver.dll`

---

### 5. `uninstall` - 卸载驱动

从系统中卸载虚拟显示驱动。

#### 语法
```bash
vddctl uninstall
```

#### 示例
```batch
vddctl uninstall
```

#### 输出
```
Uninstalling driver...
Driver uninstalled successfully.
```

#### 注意事项
- ⚠️ **需要管理员权限**
- 会移除所有 IddSampleDriver 设备
- 会清理相关注册表项

---

### 6. `activate` - 激活虚拟显示器

创建并激活虚拟显示器。

#### 语法
```bash
vddctl activate [选项]
```

#### 选项
- `--name <名称>`: 显示器名称（默认："Virtual Display"）
- `--width <宽度>`: 宽度（默认：1920）
- `--height <高度>`: 高度（默认：1080）
- `--refresh <刷新率>`: 刷新率（默认：60）
- `--count <数量>`: 创建数量（默认：1）
- `--hdr`: 启用 HDR10
- `--stereo`: 启用立体显示

#### 示例
```batch
REM 基本激活（1920x1080 @ 60Hz）
vddctl activate

REM 自定义分辨率
vddctl activate --width 2560 --height 1440 --refresh 75

REM 创建多个显示器
vddctl activate --width 1920 --height 1080 --count 2

REM 启用 HDR
vddctl activate --width 3840 --height 2160 --hdr

REM 自定义名称
vddctl activate --name "My Virtual Display" --width 1920 --height 1080
```

#### 输出
```
Activating virtual display...
Virtual display activated successfully.
Name: Virtual Display
Resolution: 1920x1080
Refresh Rate: 60Hz
Count: 1
```

---

### 7. `deactivate` - 停用虚拟显示器

移除所有虚拟显示器。

#### 语法
```bash
vddctl deactivate
```

#### 示例
```batch
vddctl deactivate
```

#### 输出
```
Deactivating virtual display...
Virtual display deactivated successfully.
```

---

### 8. `setmode` - 设置显示模式

更改虚拟显示器的分辨率和刷新率。

#### 语法
```bash
vddctl setmode --index <索引> --width <宽度> --height <高度> [--refresh <刷新率>]
```

#### 选项
- `--index <n>`: 显示器索引（0-based）
- `--width <宽度>`: 新宽度
- `--height <高度>`: 新高度
- `--refresh <刷新率>`: 新刷新率（默认：60）

#### 示例
```batch
REM 更改第一个显示器为 2560x1440 @ 60Hz
vddctl setmode --index 0 --width 2560 --height 1440

REM 更改刷新率
vddctl setmode --index 0 --width 1920 --height 1080 --refresh 120

REM 更改第二个显示器
vddctl setmode --index 1 --width 3840 --height 2160 --refresh 60
```

#### 输出
```
Setting display mode...
Display mode set successfully.
Index: 0
Resolution: 2560x1440
Refresh Rate: 75Hz
```

---

### 9. `setlocation` - 设置显示位置

设置虚拟显示器在桌面拓扑中的位置。

#### 语法
```bash
vddctl setlocation --index <索引> --x <X坐标> --y <Y坐标> --width <宽度> --height <高度>
```

#### 选项
- `--index <n>`: 显示器索引
- `--x <坐标>`: X 位置
- `--y <坐标>`: Y 位置
- `--width <宽度>`: 宽度
- `--height <高度>`: 高度

#### 示例
```batch
REM 将显示器放在主显示器右侧
vddctl setlocation --index 0 --x 1920 --y 0 --width 1920 --height 1080

REM 将显示器放在上方
vddctl setlocation --index 0 --x 0 --y -1080 --width 1920 --height 1080

REM 多显示器布局
vddctl setlocation --index 1 --x 3840 --y 0 --width 2560 --height 1440
```

#### 输出
```
Setting display location...
Display location set successfully.
Index: 0
Position: (1920, 0)
Size: 1920x1080
```

---

### 10. `setprimary` - 设置主显示器

将虚拟显示器设置为主显示器。

#### 语法
```bash
vddctl setprimary --index <索引>
```

#### 选项
- `--index <n>`: 要设为主显示器的索引

#### 示例
```batch
REM 将第一个虚拟显示器设为主显示器
vddctl setprimary --index 0

REM 将第二个虚拟显示器设为主显示器
vddctl setprimary --index 1
```

#### 输出
```
Setting primary display...
Primary display set successfully.
Index: 0
```

⚠️ **注意**: 这会影响所有应用程序的默认显示位置

---

### 11. `list` - 列出显示器

列出所有显示器和适配器信息。

#### 语法
```bash
vddctl list
```

#### 示例
```batch
vddctl list
```

#### 输出
```
Listing displays and adapters...

Adapters:
  [0] Intel(R) UHD Graphics 630
  [1] IddSampleDriver Adapter

Displays:
  [0] Generic PnP Monitor (1920x1080 @ 60Hz) - Primary
  [1] Virtual Display (1920x1080 @ 60Hz)
  [2] Virtual Display (2560x1440 @ 75Hz)
```

---

### 12. `shutdown` - 关闭 SDK

关闭 VDD SDK，释放资源。

#### 语法
```bash
vddctl shutdown
```

#### 示例
```batch
vddctl shutdown
```

#### 输出
```
Shutting down VDD SDK...
VDD SDK shutdown successfully.
```

---

### 13. `help` - 显示帮助

显示命令帮助信息。

#### 语法
```bash
vddctl help
```

#### 示例
```batch
vddctl help
```

---

## 📝 完整工作流程示例

### 示例 1: 基本安装和激活

```batch
@echo off

REM 1. 初始化 SDK
vddctl init

REM 2. 检查状态
vddctl status

REM 3. 安装驱动（需要管理员权限）
vddctl install --inf IddSampleDriver_Fixed.inf

REM 4. 激活虚拟显示器
vddctl activate --width 1920 --height 1080

REM 5. 查看显示器列表
vddctl list

REM 6. 停用虚拟显示器
vddctl deactivate

REM 7. 卸载驱动
vddctl uninstall
```

### 示例 2: 创建多显示器布局

```batch
@echo off

REM 初始化
vddctl init
vddctl install --inf IddSampleDriver_Fixed.inf

REM 创建两个虚拟显示器
vddctl activate --width 1920 --height 1080 --count 2

REM 设置第一个显示器（主显示器右侧）
vddctl setlocation --index 0 --x 1920 --y 0 --width 1920 --height 1080

REM 设置第二个显示器（第一个虚拟显示器右侧）
vddctl setlocation --index 1 --x 3840 --y 0 --width 1920 --height 1080

REM 查看布局
vddctl list
```

### 示例 3: 测试不同分辨率

```batch
@echo off

vddctl init
vddctl install --inf IddSampleDriver_Fixed.inf

REM 测试 1080p
vddctl activate --width 1920 --height 1080
timeout /t 5
vddctl deactivate

REM 测试 1440p
vddctl activate --width 2560 --height 1440
timeout /t 5
vddctl deactivate

REM 测试 4K
vddctl activate --width 3840 --height 2160
timeout /t 5
vddctl deactivate
```

---

## ⚠️ 重要注意事项

1. **管理员权限**
   - `install` 和 `uninstall` 命令需要管理员权限
   - 使用 `powershell Start-Process -Verb RunAs` 提升权限

2. **INF 文件**
   - 使用 `IddSampleDriver_Fixed.inf`（UTF-8 编码，已验证）
   - 与 `x64\Release\IddSampleDriver.dll` 配合使用
   - 不需要每次重新 build

3. **驱动状态**
   - 使用 `vddctl status` 检查当前状态
   - 使用 `vddctl list` 查看所有显示器

4. **错误处理**
   - 如果命令失败，查看输出的错误信息
   - 使用 `--verbose` 获取详细日志

---

## 🔧 故障排除

### 问题: "Driver Installed: No"

**解决方案**:
```batch
REM 确保使用管理员权限
powershell -Command "Start-Process 'vddctl.exe' -ArgumentList 'install --inf IddSampleDriver_Fixed.inf' -Verb RunAs -Wait"
```

### 问题: "Display Active: No"

**原因**: 驱动未安装或未初始化

**解决方案**:
```batch
vddctl init
vddctl install --inf IddSampleDriver_Fixed.inf
vddctl activate --width 1920 --height 1080
```

### 问题: "Failed to enumerate adapters"

**原因**: SDK 未初始化

**解决方案**:
```batch
vddctl init
```

---

## 📊 命令快速参考

| 命令 | 需要管理员 | 主要用途 |
|------|-----------|---------|
| `init` | ❌ | 初始化 SDK |
| `status` | ❌ | 查看状态 |
| `version` | ❌ | 查看版本 |
| `install` | ✅ | 安装驱动 |
| `uninstall` | ✅ | 卸载驱动 |
| `activate` | ❌ | 创建虚拟显示器 |
| `deactivate` | ❌ | 移除虚拟显示器 |
| `setmode` | ❌ | 更改分辨率 |
| `setlocation` | ❌ | 设置位置 |
| `setprimary` | ❌ | 设为主显示器 |
| `list` | ❌ | 列出显示器 |
| `shutdown` | ❌ | 关闭 SDK |
| `help` | ❌ | 显示帮助 |

