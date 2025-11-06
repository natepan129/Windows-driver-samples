# 启用 Windows 测试模式 - 安装未签名驱动

## 为什么需要测试模式？

Windows 要求所有驱动必须经过数字签名。测试模式允许安装未签名或自签名的驱动，用于开发和测试。

---

## 方法 A：启用测试模式（推荐）

### 步骤 1：启用测试签名

**以管理员身份**打开命令提示符或 PowerShell，运行：

```cmd
bcdedit /set testsigning on
```

### 步骤 2：重启电脑

```cmd
shutdown /r /t 0
```

### 步骤 3：验证测试模式

重启后，您会在桌面右下角看到"测试模式"水印：

```
Windows 10
测试模式
内部版本 xxxxx
```

### 步骤 4：安装驱动

现在可以正常安装驱动了：

```powershell
cd C:\Users\WDKRemoteUser.WIN10TESTING.000\Desktop\Windows-driver-samples\video\IndirectDisplay\IddSampleDriver

# 重新编译生成签名文件（会自动使用测试签名）
.\build_umdf_driver.bat

# 安装驱动
.\vddctl.exe install .\x64\Release\IddSampleDriver\IddSampleDriver.inf
```

---

## 方法 B：创建和使用测试证书

如果您想要更"正规"的测试签名：

### 步骤 1：创建测试证书

```cmd
makecert -r -pe -ss PrivateCertStore -n "CN=IddSampleDriver Test" ^
  -eku 1.3.6.1.5.5.7.3.3 IddSampleDriverTest.cer
```

### 步骤 2：安装证书

```cmd
certmgr /add IddSampleDriverTest.cer /s /r localMachine root
certmgr /add IddSampleDriverTest.cer /s /r localMachine trustedpublisher
```

### 步骤 3：启用测试模式并重启

```cmd
bcdedit /set testsigning on
shutdown /r /t 0
```

### 步骤 4：签名驱动

```cmd
signtool sign /v /s PrivateCertStore /n "IddSampleDriver Test" ^
  /t http://timestamp.digicert.com ^
  x64\Release\IddSampleDriver\IddSampleDriver.dll

signtool sign /v /s PrivateCertStore /n "IddSampleDriver Test" ^
  /t http://timestamp.digicert.com ^
  x64\Release\IddSampleDriver\IddSampleDriver.cat
```

---

## 关闭测试模式

完成测试后，可以关闭测试模式：

```cmd
bcdedit /set testsigning off
shutdown /r /t 0
```

**注意：** 关闭测试模式后，未签名的驱动将无法加载。

---

## 常见问题

### Q: 测试模式会影响系统安全吗？
**A:** 测试模式会降低驱动签名检查的严格程度，建议仅在开发/测试环境使用。

### Q: 测试模式水印能去掉吗？
**A:** 官方不支持。这是 Windows 的设计，提醒用户当前处于测试模式。

### Q: 企业环境能使用测试模式吗？
**A:** 可以，但可能需要 IT 管理员权限。某些企业策略可能禁止测试模式。

### Q: 测试模式下性能会受影响吗？
**A:** 不会。测试模式只影响驱动签名验证，不影响性能。

---

## 验证测试模式状态

检查测试模式是否已启用：

```cmd
bcdedit /enum {current}
```

查找输出中的：
```
testsigning             Yes
```

---

## 故障排除

### 问题：`bcdedit` 返回"拒绝访问"
**解决：** 必须以管理员身份运行命令提示符。

### 问题：启用测试模式后无法启动
**解决：** 进入安全模式，运行 `bcdedit /set testsigning off`

### 问题：驱动仍然无法安装
**解决：** 
1. 确认已重启电脑
2. 检查是否看到"测试模式"水印
3. 确认 INF 文件中有 `CatalogFile` 引用
4. 重新编译驱动生成 `.cat` 文件

---

## 快速设置脚本

创建 `enable_test_mode.bat`：

```cmd
@echo off
echo ========================================
echo 启用 Windows 测试模式
echo ========================================
echo.
echo 这将允许安装未签名的驱动程序
echo.
pause

bcdedit /set testsigning on

if %errorLevel% EQU 0 (
    echo.
    echo ========================================
    echo 测试模式已启用！
    echo 需要重启电脑才能生效
    echo ========================================
    echo.
    choice /C YN /M "现在重启电脑"
    if errorlevel 2 goto :end
    shutdown /r /t 5
) else (
    echo.
    echo ========================================
    echo 错误：无法启用测试模式
    echo 请以管理员身份运行此脚本
    echo ========================================
)

:end
pause
```

---

## 下一步

启用测试模式并重启后：

1. 重新编译驱动（会生成 `.cat` 文件）
2. 运行 `.\vddctl.exe install`
3. 测试虚拟显示功能

完成！








