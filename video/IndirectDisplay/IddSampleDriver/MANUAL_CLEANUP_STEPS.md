# 手动清理步骤

## 🎯 问题
系统中有两个设备：
- `ROOT\IDDSAMPLEDRIVER\0000` - 旧的，无 Display Class
- `ROOT\IDDSAMPLEDRIVER\0001` - 新的，有 Display Class ✅

## ✅ 方法 1：使用管理员 PowerShell（推荐）

```powershell
# 1. 右键 "Windows PowerShell" → "以管理员身份运行"

# 2. 删除旧设备
pnputil /remove-device "ROOT\IDDSAMPLEDRIVER\0000"

# 3. 验证
Get-PnpDevice | Where-Object { $_.InstanceId -like '*IddSampleDriver*' } | Format-Table FriendlyName, Status, Class, InstanceId -AutoSize
```

**预期结果**：只剩下 `ROOT\IDDSAMPLEDRIVER\0001`，有 Display Class

---

## ✅ 方法 2：使用批处理脚本

```batch
# 右键 full_test_fixed_inf.bat → "以管理员身份运行"
```

这个脚本会：
1. 清理所有旧设备
2. 重新安装（用修复后的 INF）
3. 验证结果

---

## ✅ 方法 3：Device Manager 手动删除

1. 打开设备管理器（`devmgmt.msc`）
2. 查看 → 显示隐藏的设备
3. 找到 "IddSampleDriver Device"
4. 右键 → 卸载设备
5. ✓ 勾选 "删除此设备的驱动程序软件"
6. 确定

---

## 🧪 验证成功标准

运行此命令：
```powershell
Get-PnpDevice | Where-Object { $_.InstanceId -like '*IddSampleDriver*' } | Format-Table FriendlyName, Status, Class, ClassGuid -AutoSize
```

**期望输出**：
```
FriendlyName           Status Class   ClassGuid
IddSampleDriver Device OK     Display {4d36e968-e325-11ce-bfc1-08002be10318}
```

✅ 有 `Class: Display`
✅ 有正确的 `ClassGuid`
✅ `Status: OK`

