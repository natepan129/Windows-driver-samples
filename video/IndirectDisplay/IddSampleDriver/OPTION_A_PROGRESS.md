# 选项 A 进展报告 - vddctl 调试

## 📊 当前状态

### ✅ 已完成

1. **vddctl.exe 编译成功**
   - 使用静态链接方式
   - 位置：`build\bin\Release\vddctl.exe`
   - 大小：约 500KB

2. **基本命令工作正常**
   ```bash
   .\build\bin\Release\vddctl.exe version  # ✅ 工作
   .\build\bin\Release\vddctl.exe init     # ✅ 工作
   .\build\bin\Release\vddctl.exe status   # ✅ 工作
   ```

3. **添加了调试输出**
   - 修改 `vddctl.cpp::cmdInstall()` 函数
   - 添加详细的路径和错误信息
   - 代码已修改，待重新编译

### ⚠️ 待修复问题

1. **vddctl install 不工作** (TODO #16)
   - 命令执行但没有创建设备
   - 原因未明

2. **设备缺少 Display Class** (TODO #17)
   - `install_driver.exe` 创建的设备没有 Class
   - 这是旧问题重现

---

## 🔍 调试计划

### 下一步操作

#### 1. 重新编译带调试输出的 vddctl

```bash
cmd /c build_vddctl_static.bat
```

**预期结果**：编译成功，生成新的 `vddctl.exe`

#### 2. 用调试版本测试安装

```bash
# 先卸载旧设备
Start-Process -FilePath ".\uninstall_driver.exe" -Verb RunAs -Wait

# 用 vddctl 安装（会显示详细信息）
Start-Process -FilePath ".\build\bin\Release\vddctl.exe" -ArgumentList "install --inf IddSampleDriver_Fixed.inf" -Verb RunAs -Wait
```

**预期输出**（调试信息）：
```
Installing driver...
========================================
INF Path (raw): IddSampleDriver_Fixed.inf
INF Path (wide): IddSampleDriver_Fixed.inf
Absolute Path: C:\...\IddSampleDriver_Fixed.inf
File exists: OK
----------------------------------------
Calling InstallDriver()...
[SetupAPI 详细步骤]
----------------------------------------
Driver installed successfully.
========================================
```

#### 3. 如果仍然失败，对比两种方法

**方法 A** (vddctl):
```bash
Start-Process vddctl.exe install --inf IddSampleDriver_Fixed.inf -Verb RunAs
```

**方法 B** (install_driver.exe - 已知可工作):
```bash
Start-Process install_driver.exe IddSampleDriver_Fixed.inf -Verb RunAs
```

**对比点**：
- 工作目录
- INF 路径解析
- SetupAPI 调用顺序
- 错误处理

---

## 📋 可能的根本原因

### 假设 1: 工作目录问题

**症状**：以管理员身份运行时，工作目录可能变成 `C:\Windows\system32`

**测试**：
```cpp
// 在 cmdInstall() 开头添加
wchar_t cwd[MAX_PATH];
GetCurrentDirectoryW(MAX_PATH, cwd);
std::wcout << L"Current Directory: " << cwd << std::endl;
```

**解决方案**：使用绝对路径

### 假设 2: 参数解析问题

**症状**：`--inf` 参数可能没有正确解析

**测试**：已添加 `INF Path (raw)` 输出

**解决方案**：如果路径不对，修复 ArgumentParser

### 假设 3: 权限问题

**症状**：SetupAPI 需要特定权限

**测试**：
```cpp
// 检查是否以管理员运行
BOOL isAdmin = IsUserAnAdmin();
std::cout << "Running as Admin: " << (isAdmin ? "Yes" : "No") << std::endl;
```

**解决方案**：确保以管理员身份运行

### 假设 4: SetupAPI 调用顺序问题

**症状**：`vddsdk.cpp::InstallDriver()` 的某些步骤失败

**测试**：在 `InstallDriver()` 函数中添加更多日志

**解决方案**：修复 SetupAPI 调用

---

## 🛠️ 快速修复方案

如果调试困难，可以采用以下临时方案：

### 方案 1: vddctl 调用 install_driver.exe

在 `vddctl.cpp::cmdInstall()` 中：

```cpp
void cmdInstall(const ArgumentParser& args) {
    std::cout << "Installing driver..." << std::endl;
    
    std::string infPath = args.getOption("inf", "IddSampleDriver.inf");
    
    // 临时方案：调用 install_driver.exe
    std::wstring cmd = L"install_driver.exe " + std::wstring(infPath.begin(), infPath.end());
    
    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    
    if (CreateProcessW(nullptr, const_cast<wchar_t*>(cmd.c_str()), 
                       nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi)) {
        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        std::cout << "Driver installed successfully." << std::endl;
    } else {
        std::cout << "Failed to install driver." << std::endl;
    }
}
```

**优点**：立即可用
**缺点**：不是长期解决方案

### 方案 2: 使用绝对路径

在调用 `InstallDriver()` 前转换为绝对路径：

```cpp
// Get absolute path
wchar_t absPath[MAX_PATH];
GetFullPathNameW(winfPath.c_str(), MAX_PATH, absPath, nullptr);

// Use absolute path
Status status = InstallDriver(absPath);
```

---

## 📝 测试检查清单

完成以下测试后，问题应该被定位：

- [ ] 重新编译 vddctl
- [ ] 测试 `vddctl install` 并查看调试输出
- [ ] 检查 INF 路径是否正确解析
- [ ] 检查工作目录
- [ ] 检查是否以管理员运行
- [ ] 对比 `install_driver.exe` 的输出
- [ ] 如果路径正确但仍失败，添加 `InstallDriver()` 内部日志
- [ ] 测试使用绝对路径
- [ ] 检查设备创建结果

---

## 🎯 成功标准

### 阶段 1 完成标准

- [ ] `vddctl install --inf IddSampleDriver_Fixed.inf` 成功创建设备
- [ ] 设备出现在 Device Manager
- [ ] 设备有正确的 Display Class
- [ ] 设备状态为 OK
- [ ] `vddctl status` 显示 "Driver Installed: Yes"

### 最终目标

完成后，用户应该能够：

```bash
# 完整工作流程
vddctl init
vddctl install --inf IddSampleDriver_Fixed.inf
vddctl status  # 显示 "Driver Installed: Yes"
vddctl activate --width 1920 --height 1080
vddctl list
vddctl deactivate
vddctl uninstall
```

---

## 📞 下一步行动

**立即可执行**：

```bash
# 1. 重新编译
cmd /c build_vddctl_static.bat

# 2. 清理旧设备
Start-Process -FilePath ".\uninstall_driver.exe" -Verb RunAs -Wait

# 3. 测试安装（会显示详细调试信息）
Start-Process -FilePath ".\build\bin\Release\vddctl.exe" -ArgumentList "install --inf IddSampleDriver_Fixed.inf" -Verb RunAs -Wait

# 4. 检查结果
.\build\bin\Release\vddctl.exe status
powershell -Command "Get-PnpDevice | Where-Object { $_.FriendlyName -like '*IddSampleDriver*' } | Format-Table -AutoSize"
```

**如果成功**：
- 继续测试其他命令（activate, deactivate）
- 进入阶段 2（安全机制）

**如果失败**：
- 分析调试输出
- 根据输出信息采取对应措施
- 可能需要添加更多内部日志



