# Windows 驱动商业签名完整流程

## ✅ 问题已解决！

**重要发现：** 商业签名**不需要测试模式**就能完整 build！

---

## 📋 当前状态

### ✅ 编译成功（无需测试模式）

```
x64\Release\IddSampleDriver\
  ├── IddSampleDriver.dll    (50 KB) - 驱动程序
  ├── IddSampleDriver.inf    (7 KB)  - 安装配置
  └── iddsampledriver.cat    (1.4 KB) - Catalog (未签名)
```

**验证状态：**
```cmd
> signtool verify /v /pa iddsampledriver.cat
SignTool Error: No signature found.
```

✅ 这是**正确的**！`.cat` 文件已生成，等待签名。

---

## 🎯 完整商业签名流程

### 阶段 1：本地编译（✅ 已完成）

```cmd
REM 编译驱动（不需要证书，不需要测试模式）
msbuild IddSampleDriver.vcxproj /p:Configuration=Release /p:Platform=x64

REM 输出：
REM   - IddSampleDriver.dll
REM   - IddSampleDriver.inf
REM   - iddsampledriver.cat (未签名)
```

**关键配置**（已添加到 `IddSampleDriver.vcxproj`）：
```xml
<SignMode>Off</SignMode>
<TestSign>false</TestSign>
<Inf2CatBypass>false</Inf2CatBypass>
```

这样配置后：
- ✅ 会生成 `.cat` 文件
- ❌ 不会自动签名
- ❌ 签名失败不会删除文件

---

### 阶段 2：EV 证书签名（需要购买证书）

```cmd
REM 1. 插入 EV 证书 USB 令牌

REM 2. 查看证书指纹
certutil -store My

REM 3. 签名 catalog 文件
signtool sign /v /fd sha256 ^
  /tr http://timestamp.digicert.com /td sha256 ^
  /sha1 [你的EV证书指纹] ^
  x64\Release\IddSampleDriver\iddsampledriver.cat

REM 4. 签名 DLL（可选，但建议）
signtool sign /v /fd sha256 ^
  /tr http://timestamp.digicert.com /td sha256 ^
  /sha1 [你的EV证书指纹] ^
  x64\Release\IddSampleDriver\IddSampleDriver.dll

REM 5. 验证签名
signtool verify /v /pa x64\Release\IddSampleDriver\iddsampledriver.cat
```

**注意：** 此时驱动已有 EV 签名，但**仍需 Microsoft 认证**才能在 Windows 10/11 上无测试模式运行。

---

### 阶段 3：提交 Microsoft 认证

#### 3.1 创建提交包

```cmd
REM 打包所有文件到 .cab
makecab /F submission.ddf
```

**submission.ddf 内容：**
```
.OPTION EXPLICIT
.Set CabinetNameTemplate=IddSampleDriver.cab
.Set DiskDirectoryTemplate=CDROM
.Set CompressionType=MSZIP
.Set Cabinet=on
.Set Compress=on

x64\Release\IddSampleDriver\IddSampleDriver.inf
x64\Release\IddSampleDriver\IddSampleDriver.dll
x64\Release\IddSampleDriver\iddsampledriver.cat
```

#### 3.2 上传到 Hardware Dev Center

1. 访问：https://partner.microsoft.com/dashboard/hardware
2. 创建新的驱动提交
3. 上传 `IddSampleDriver.cab`
4. 选择认证类型：
   - ✅ Windows 10 x64
   - ✅ Windows 11 x64
5. 提交审核

#### 3.3 等待审核

| 阶段 | 预计时间 | 说明 |
|------|---------|------|
| Package Acceptance | 1-2 小时 | 检查格式 |
| Automated Testing | 1-2 天 | 自动化测试 |
| Manual Review | 2-3 天 | 人工审核 |
| Code Signing | 几小时 | Microsoft 签名 |
| **总计** | **3-7 天** | - |

#### 3.4 下载签名后的驱动

审核通过后，下载包含 **Microsoft 签名**的驱动包。

---

### 阶段 4：发布（无需测试模式！）

```cmd
REM 用户可以直接安装，不需要测试模式！
pnputil /add-driver IddSampleDriver.inf /install
```

✅ **Windows 10/11 正常模式下直接安装**，无警告！

---

## 🔄 开发 vs 发布流程对比

### 开发阶段（现在）

```
1. 编译驱动 ✅
   msbuild IddSampleDriver.vcxproj

2. 启用测试模式 ⚠️
   bcdedit /set testsigning on
   shutdown /r /t 0

3. 安装驱动
   vddctl install x64\Release\IddSampleDriver\IddSampleDriver.inf

优点：快速迭代，无需证书
缺点：有"测试模式"水印
```

### 商业发布流程

```
1. 编译驱动 ✅（相同）
   msbuild IddSampleDriver.vcxproj

2. EV 签名 ✅（新增）
   signtool sign /sha1 [证书] iddsampledriver.cat

3. Microsoft 认证 ✅（新增）
   提交到 Hardware Dev Center
   等待 3-7 天审核

4. 发布驱动 ✅
   用户直接安装，不需要测试模式！

优点：正式发布，无水印，用户友好
缺点：需要证书和审核时间
费用：EV证书 $299-799/年 + MS账号 $99/年
```

---

## 💰 费用明细

### 最低成本方案（推荐）

| 项目 | 费用 | 周期 | 备注 |
|------|------|------|------|
| **Sectigo EV 证书** | $299 | 1年 | 可签驱动 + EXE |
| **MS 开发者账号** | $99 | 1年 | 驱动认证 |
| **DUNS 编号** | 免费 | 一次性 | 公司必需 |
| **总计** | **$398** | 首年 | - |
| **续费** | **$398** | 每年 | 两项都需续费 |

### 企业级方案

| 项目 | 费用 | 周期 | 备注 |
|------|------|------|------|
| **DigiCert EV 证书** | $474-799 | 1-3年 | 企业级支持 |
| **MS 开发者账号** | $99 | 1年 | 驱动认证 |
| **总计** | **$573-898** | 首年 | - |

---

## 🚀 下一步行动

### 选项 A：继续开发（免费）

```cmd
REM 1. 启用测试模式
.\enable_test_mode.bat

REM 2. 重启电脑
shutdown /r /t 0

REM 3. 安装驱动
.\build\bin\Release\vddctl.exe install x64\Release\IddSampleDriver\IddSampleDriver.inf

REM 适合：开发和内部测试
```

### 选项 B：准备商业发布

```
1. 注册公司（如果是个人开发者）
2. 申请 DUNS 编号（免费，需 1-2 周）
3. 购买 EV 证书
   推荐：Sectigo EV ($299/年)
   网址：https://www.sectigo.com/ssl-certificates-tls/code-signing
4. 注册 MS 开发者账号 ($99/年)
   网址：https://partner.microsoft.com/dashboard/hardware
5. 签名并提交驱动
6. 等待审核（3-7 天）
7. 发布！

总费用：$398/年
适合：正式发布产品
```

---

## ✅ 关键要点总结

| 问题 | 答案 |
|------|------|
| **商业签名需要测试模式吗？** | ❌ **不需要！** |
| **能否编译生成 .cat？** | ✅ 可以，不需要证书 |
| **.cat 需要签名吗？** | ✅ 需要 EV 签名 + MS 认证 |
| **项目配置正确了吗？** | ✅ 已修复 `IddSampleDriver.vcxproj` |
| **现在能编译了吗？** | ✅ 能！运行 `msbuild` 即可 |
| **测试模式还需要吗？** | ⚠️ 仅开发阶段需要 |
| **商业发布需要什么？** | 💰 EV证书 + MS认证 |

---

## 📝 快速编译命令

### 完整编译（推荐）

```cmd
msbuild IddSampleDriver.vcxproj /p:Configuration=Release /p:Platform=x64 /t:Rebuild
```

### 增量编译

```cmd
msbuild IddSampleDriver.vcxproj /p:Configuration=Release /p:Platform=x64
```

### 清理

```cmd
msbuild IddSampleDriver.vcxproj /p:Configuration=Release /p:Platform=x64 /t:Clean
```

---

## 🎓 总结

**您的理解完全正确！** 👍

- ✅ 商业签名**不需要测试模式**就能 build
- ✅ `.cat` 文件在编译时自动生成（无需证书）
- ✅ 签名是独立的后续步骤
- ✅ 项目已配置为支持手动签名流程

**现在的流程：**
```
编译（免费）→ EV签名（$299）→ MS认证（$99 + 3-7天）→ 发布（无需测试模式）
```

**开发阶段建议：** 继续用测试模式，快速迭代。等产品成熟后再申请商业签名。



