# 如何运行 vddctl 命令

## ❌ 常见错误

### PowerShell 中的路径问题

**错误命令** (PowerShell):
```powershell
build\bin\Release\vddctl.exe init
# Error: The module 'build' could not be loaded
```

---

## ✅ 正确的运行方法

### 方法 1: PowerShell 中使用 `.\` 前缀

```powershell
.\build\bin\Release\vddctl.exe init
.\build\bin\Release\vddctl.exe status
.\build\bin\Release\vddctl.exe install --inf IddSampleDriver_Fixed.inf
```

### 方法 2: 使用 CMD (命令提示符)

```batch
build\bin\Release\vddctl.exe init
build\bin\Release\vddctl.exe status
build\bin\Release\vddctl.exe install --inf IddSampleDriver_Fixed.inf
```

### 方法 3: 使用完整路径

```powershell
# PowerShell
C:\Users\...\IddSampleDriver\build\bin\Release\vddctl.exe init

# CMD
C:\Users\...\IddSampleDriver\build\bin\Release\vddctl.exe init
```

### 方法 4: 先 CD 到目录

```batch
cd build\bin\Release
vddctl.exe init
vddctl.exe status
```

### 方法 5: 使用 & 操作符 (PowerShell)

```powershell
& "build\bin\Release\vddctl.exe" init
& "build\bin\Release\vddctl.exe" status
```

---

## 🚀 推荐方法

### 在 CMD 中运行 (最简单)

```batch
REM 打开 CMD (不是 PowerShell)
REM 进入项目目录
cd C:\Users\WDKRemoteUser.WIN10TESTING.000\Desktop\Windows-driver-samples\video\IndirectDisplay\IddSampleDriver

REM 直接运行命令
build\bin\Release\vddctl.exe init
build\bin\Release\vddctl.exe install --inf IddSampleDriver_Fixed.inf
build\bin\Release\vddctl.exe status
```

### 使用提供的批处理脚本

```batch
REM 测试初始化
cmd /c test_vddctl_init.bat

REM 安装驱动
cmd /c install_with_vddctl.bat

REM 完整测试
cmd /c test_vddctl_final.bat
```

---

## 📋 快速参考

| 环境 | 命令格式 | 示例 |
|------|---------|------|
| CMD | `路径\vddctl.exe 命令` | `build\bin\Release\vddctl.exe init` |
| PowerShell | `.\路径\vddctl.exe 命令` | `.\build\bin\Release\vddctl.exe init` |
| PowerShell (& 操作符) | `& "路径\vddctl.exe" 命令` | `& "build\bin\Release\vddctl.exe" init` |
| 完整路径 | `C:\完整路径\vddctl.exe 命令` | `C:\...\vddctl.exe init` |

---

## 🎯 立即可用的命令 (复制粘贴)

### 如果你在 PowerShell:

```powershell
# 初始化
.\build\bin\Release\vddctl.exe init

# 查看状态
.\build\bin\Release\vddctl.exe status

# 查看版本
.\build\bin\Release\vddctl.exe version

# 安装驱动 (需要管理员)
Start-Process ".\build\bin\Release\vddctl.exe" -ArgumentList "install --inf IddSampleDriver_Fixed.inf" -Verb RunAs -Wait

# 激活显示器
.\build\bin\Release\vddctl.exe activate --width 1920 --height 1080

# 列出显示器
.\build\bin\Release\vddctl.exe list
```

### 如果你在 CMD:

```batch
REM 初始化
build\bin\Release\vddctl.exe init

REM 查看状态
build\bin\Release\vddctl.exe status

REM 查看版本
build\bin\Release\vddctl.exe version

REM 安装驱动 (会弹出 UAC)
build\bin\Release\vddctl.exe install --inf IddSampleDriver_Fixed.inf

REM 激活显示器
build\bin\Release\vddctl.exe activate --width 1920 --height 1080

REM 列出显示器
build\bin\Release\vddctl.exe list
```

---

## 💡 最简单的方法

**创建一个快捷脚本** `vdd.bat`:

```batch
@echo off
build\bin\Release\vddctl.exe %*
```

然后就可以简单地运行：

```batch
vdd init
vdd status
vdd install --inf IddSampleDriver_Fixed.inf
vdd activate --width 1920 --height 1080
vdd list
```

