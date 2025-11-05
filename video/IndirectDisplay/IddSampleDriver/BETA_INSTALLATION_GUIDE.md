# IddSampleDriver Beta 安装指南

## ⚠️ 重要提示
本驱动为测试版本，未经过 Microsoft WHQL 认证。

## 安装步骤

### 方法 A：自动安装（推荐）

1. **以管理员身份运行** `test_driver_fix.bat`
2. 按照提示完成安装
3. 驱动会自动处理未签名问题

### 方法 B：手动安装

1. 以管理员身份打开 PowerShell
2. 运行以下命令：
   ```powershell
   .\vddctl.exe install .\x64\Release\IddSampleDriver.inf
   ```
3. 如果看到签名警告，选择"仍然安装"

## 常见问题

### Q: 为什么会显示"未验证发布者"警告？
**A:** 本驱动使用自签名，未经过 Microsoft 官方认证。这是正常的开发/测试版本。

### Q: Windows Defender 报告为恶意软件怎么办？
**A:** 这是误报。可以添加到白名单：
- Windows 安全中心 → 病毒和威胁防护 → 管理设置 → 添加排除项

### Q: 企业环境无法安装怎么办？
**A:** 请联系 IT 管理员，可能需要：
1. 禁用驱动签名强制（不推荐）
2. 将驱动添加到企业白名单
3. 等待正式签名版本

## 卸载

```powershell
.\vddctl.exe uninstall
```

## 技术支持

如有问题，请提供以下信息：
- Windows 版本
- `.\vddctl.exe status` 输出
- 安装时的错误信息




