# VDD 驱动安装测试指南

## 当前状态

✅ **InstallDriver 实现完成**  
✅ **调试输出已添加**  
✅ **前 22 步全部成功**  
❌ **Step 23 失败：驱动签名问题**

## 错误详情

```
[VDD] Step 23: Installing device (this may take a while)...
[VDD] ERROR: DIF_INSTALLDEVICE failed, error=-536870377
```

**错误码：** `-536870377` (0xE0000227)  
**原因：** Windows 驱动签名验证失败

## 解决步骤

### 步骤 1：启用测试签名模式

以管理员身份运行：
```batch
fix_driver_signing.bat
```

这个脚本会：
1. 检查当前测试签名状态
2. 启用测试签名模式 (`bcdedit /set testsigning on`)
3. 禁用完整性检查 (`bcdedit /set nointegritychecks on`)
4. 提示重启

### 步骤 2：重启电脑

**重要：** 必须重启才能生效！

重启后你会看到：
- 屏幕右下角显示 **"测试模式"** 水印
- 这是正常的，表示测试签名已启用

### 步骤 3：重新测试安装

重启后，以管理员身份运行：
```batch
test_install.bat
```

应该看到：
```
[VDD] Step 23: Installing device (this may take a while)...
[VDD] Step 24: Device installed successfully!
[VDD] ========================================
[VDD] SUCCESS: Driver installed
[VDD] ========================================
```

### 步骤 4：验证安装

```batch
vddctl.exe status
```

应该显示：
```
Driver Installed: Yes
```

## 如果还是失败

### 检查驱动文件

确认以下文件存在：
```
x64\Release\IddSampleDriver\
  ├── IddSampleDriver.inf
  ├── IddSampleDriver.dll
  └── iddsampledriver.cat
```

### 查看详细错误

安装失败时，查看：
- 设备管理器 → 查看 → 显示隐藏的设备
- 查找 "IddSampleDriver" 
- 右键 → 属性 → 查看错误详情

### 查看安装日志

```powershell
Get-Content C:\Windows\INF\setupapi.dev.log | Select-String -Context 5,5 "IddSampleDriver"
```

## 调试输出说明

成功的安装会显示 24 步：

1. ✅ Validation passed
2. ✅ Absolute path resolved
3. ✅ File exists
4. ✅ Running as Administrator
5. ✅ Checking for existing device
6. ✅ No existing device found
7. ✅ Staging INF to Driver Store
8. ✅ INF staged successfully
9. ✅ Creating device info list
10. ✅ Device info list created
11. ✅ Creating device info
12. ✅ Device info created
13. ✅ Setting Hardware ID
14. ✅ Hardware ID set
15. ✅ Registering device
16. ✅ Device registered
17. ✅ Setting install parameters
18. ✅ Install parameters set
19. ✅ Building driver list
20. ✅ Driver list built
21. ✅ Selecting best compatible driver
22. ✅ Driver selected
23. ✅ Installing device
24. ✅ **Device installed successfully!**

## 回滚机制

如果安装失败，代码会自动回滚：
- 清理驱动信息列表
- 移除部分创建的设备节点
- 恢复到安装前状态

## 下一步

安装成功后，可以测试：

1. **卸载驱动**
   ```batch
   vddctl.exe uninstall
   ```

2. **激活显示器**
   ```batch
   vddctl.exe activate
   ```

3. **查看状态**
   ```batch
   vddctl.exe status
   ```

## 故障排除

| 问题 | 解决方案 |
|------|---------|
| 测试签名未启用 | 运行 `fix_driver_signing.bat` 并重启 |
| 驱动文件缺失 | 重新编译 UMDF 驱动 |
| 权限不足 | 以管理员身份运行 |
| 设备已存在 | 先运行 `vddctl.exe uninstall` |

## 技术细节

### 使用的 SetupAPI 函数
- `DiInstallDriverW` - 暂存 INF 到驱动存储
- `SetupDiCreateDeviceInfoList` - 创建设备信息列表
- `SetupDiCreateDeviceInfoW` - 创建设备信息
- `SetupDiSetDeviceRegistryPropertyW` - 设置硬件 ID
- `SetupDiCallClassInstaller` - 调用类安装器
- `SetupDiBuildDriverInfoList` - 构建驱动列表
- `DIF_REGISTERDEVICE` - 注册设备
- `DIF_SELECTBESTCOMPATDRV` - 选择最佳兼容驱动
- `DIF_INSTALLDEVICE` - 安装设备

### 硬件 ID
```
ROOT\IddSampleDriver
```

### 设备类 GUID
```
{4D36E968-E325-11CE-BFC1-08002BE10318}
```
Display Adapter Class

---

**当前版本：** 2025-11-03  
**状态：** 等待测试签名启用并重启测试

# VDD 驱动安装测试指南

## 当前状态

✅ **InstallDriver 实现完成**  
✅ **调试输出已添加**  
✅ **前 22 步全部成功**  
❌ **Step 23 失败：驱动签名问题**

## 错误详情

```
[VDD] Step 23: Installing device (this may take a while)...
[VDD] ERROR: DIF_INSTALLDEVICE failed, error=-536870377
```

**错误码：** `-536870377` (0xE0000227)  
**原因：** Windows 驱动签名验证失败

## 解决步骤

### 步骤 1：启用测试签名模式

以管理员身份运行：
```batch
fix_driver_signing.bat
```

这个脚本会：
1. 检查当前测试签名状态
2. 启用测试签名模式 (`bcdedit /set testsigning on`)
3. 禁用完整性检查 (`bcdedit /set nointegritychecks on`)
4. 提示重启

### 步骤 2：重启电脑

**重要：** 必须重启才能生效！

重启后你会看到：
- 屏幕右下角显示 **"测试模式"** 水印
- 这是正常的，表示测试签名已启用

### 步骤 3：重新测试安装

重启后，以管理员身份运行：
```batch
test_install.bat
```

应该看到：
```
[VDD] Step 23: Installing device (this may take a while)...
[VDD] Step 24: Device installed successfully!
[VDD] ========================================
[VDD] SUCCESS: Driver installed
[VDD] ========================================
```

### 步骤 4：验证安装

```batch
vddctl.exe status
```

应该显示：
```
Driver Installed: Yes
```

## 如果还是失败

### 检查驱动文件

确认以下文件存在：
```
x64\Release\IddSampleDriver\
  ├── IddSampleDriver.inf
  ├── IddSampleDriver.dll
  └── iddsampledriver.cat
```

### 查看详细错误

安装失败时，查看：
- 设备管理器 → 查看 → 显示隐藏的设备
- 查找 "IddSampleDriver" 
- 右键 → 属性 → 查看错误详情

### 查看安装日志

```powershell
Get-Content C:\Windows\INF\setupapi.dev.log | Select-String -Context 5,5 "IddSampleDriver"
```

## 调试输出说明

成功的安装会显示 24 步：

1. ✅ Validation passed
2. ✅ Absolute path resolved
3. ✅ File exists
4. ✅ Running as Administrator
5. ✅ Checking for existing device
6. ✅ No existing device found
7. ✅ Staging INF to Driver Store
8. ✅ INF staged successfully
9. ✅ Creating device info list
10. ✅ Device info list created
11. ✅ Creating device info
12. ✅ Device info created
13. ✅ Setting Hardware ID
14. ✅ Hardware ID set
15. ✅ Registering device
16. ✅ Device registered
17. ✅ Setting install parameters
18. ✅ Install parameters set
19. ✅ Building driver list
20. ✅ Driver list built
21. ✅ Selecting best compatible driver
22. ✅ Driver selected
23. ✅ Installing device
24. ✅ **Device installed successfully!**

## 回滚机制

如果安装失败，代码会自动回滚：
- 清理驱动信息列表
- 移除部分创建的设备节点
- 恢复到安装前状态

## 下一步

安装成功后，可以测试：

1. **卸载驱动**
   ```batch
   vddctl.exe uninstall
   ```

2. **激活显示器**
   ```batch
   vddctl.exe activate
   ```

3. **查看状态**
   ```batch
   vddctl.exe status
   ```

## 故障排除

| 问题 | 解决方案 |
|------|---------|
| 测试签名未启用 | 运行 `fix_driver_signing.bat` 并重启 |
| 驱动文件缺失 | 重新编译 UMDF 驱动 |
| 权限不足 | 以管理员身份运行 |
| 设备已存在 | 先运行 `vddctl.exe uninstall` |

## 技术细节

### 使用的 SetupAPI 函数
- `DiInstallDriverW` - 暂存 INF 到驱动存储
- `SetupDiCreateDeviceInfoList` - 创建设备信息列表
- `SetupDiCreateDeviceInfoW` - 创建设备信息
- `SetupDiSetDeviceRegistryPropertyW` - 设置硬件 ID
- `SetupDiCallClassInstaller` - 调用类安装器
- `SetupDiBuildDriverInfoList` - 构建驱动列表
- `DIF_REGISTERDEVICE` - 注册设备
- `DIF_SELECTBESTCOMPATDRV` - 选择最佳兼容驱动
- `DIF_INSTALLDEVICE` - 安装设备

### 硬件 ID
```
ROOT\IddSampleDriver
```

### 设备类 GUID
```
{4D36E968-E325-11CE-BFC1-08002BE10318}
```
Display Adapter Class

---

**当前版本：** 2025-11-03  
**状态：** 等待测试签名启用并重启测试

