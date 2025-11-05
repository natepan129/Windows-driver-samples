# Windows 驱动正统签名完整指南

## 📋 目录

1. [签名类型对比](#签名类型对比)
2. [正统签名流程](#正统签名流程)
3. [EV 代码签名证书购买](#ev-代码签名证书购买)
4. [Microsoft 硬件开发者中心认证](#microsoft-硬件开发者中心认证)
5. [测试签名（开发阶段）](#测试签名开发阶段)
6. [费用对比](#费用对比)

---

## 签名类型对比

| 签名类型 | 适用场景 | Windows 10/11 支持 | 需要测试模式 | 费用 |
|---------|---------|-------------------|-------------|------|
| **Microsoft 认证签名** ✅ | 正式发布 | ✅ 完全支持 | ❌ 不需要 | $$$ |
| **EV 代码签名** | 正式发布 | ⚠️ 需 MS 认证 | ❌ 不需要 | $$$ |
| **标准代码签名** | 旧版系统 | ❌ Win10+ 不支持 | - | $$ |
| **自签名/测试签名** 🔧 | 开发测试 | ⚠️ 仅测试模式 | ✅ 需要 | 免费 |

---

## 正统签名流程

### 🎯 **完整流程图**

```
第 1 步: 购买 EV 代码签名证书
   ↓
第 2 步: 注册 Microsoft 硬件开发者中心账号
   ↓
第 3 步: 用 EV 证书签名驱动包
   ↓
第 4 步: 提交驱动到 Microsoft Hardware Dev Center
   ↓
第 5 步: Microsoft 审核和测试
   ↓
第 6 步: 获得 Microsoft 签名的驱动
   ↓
第 7 步: 发布驱动（可通过 Windows Update）
```

---

## EV 代码签名证书购买

### 1️⃣ **什么是 EV 证书？**

**Extended Validation (EV) 代码签名证书**是最高级别的代码签名证书：
- 需要严格的身份验证（企业营业执照、地址证明等）
- 存储在 **USB 硬件令牌**中，更安全
- 是提交驱动到 Microsoft 的**必要条件**

### 2️⃣ **支持的证书颁发机构 (CA)**

Microsoft 认可的 CA 包括：

| CA 名称 | 价格（USD/年） | 网址 | 备注 |
|---------|---------------|------|------|
| **DigiCert** | $474 - $799 | https://www.digicert.com | 最常用，服务好 |
| **Sectigo (Comodo)** | $299 - $415 | https://www.sectigo.com | 性价比高 |
| **GlobalSign** | $399 - $599 | https://www.globalsign.com | 国际认可度高 |
| **Entrust** | $599 | https://www.entrust.com | 企业级 |

### 3️⃣ **购买 EV 证书所需材料**

#### **公司申请：**
- 营业执照（Business License）
- 公司注册文件（Articles of Incorporation）
- 地址证明（Utility Bill、Bank Statement）
- 联系人身份证明（护照/身份证）
- 电话号码（可公开查询的）
- DUNS 编号（可选，但能加速审核）

#### **个人开发者：**
- ⚠️ **重要：Microsoft 硬件开发者中心现在要求公司账号**
- 个人开发者**无法**直接提交驱动认证
- 建议成立小型公司或使用代签名服务

### 4️⃣ **购买流程**

1. **选择 CA** → 访问官网，选择 "EV Code Signing Certificate"
2. **提交申请** → 填写公司信息
3. **身份验证** → CA 会电话/邮件核实（需 3-7 个工作日）
4. **生成证书** → 审核通过后，CA 会邮寄 **USB 令牌**
5. **安装证书** → 插入 USB 令牌，按说明安装

---

## Microsoft 硬件开发者中心认证

### 1️⃣ **注册硬件开发者账号**

访问：https://partner.microsoft.com/dashboard/hardware

#### **账号类型：**
- **公司账号**（Company Account）：**必需**，个人账号无法提交驱动
- 年费：**$99 USD**（首次 $119）

#### **注册要求：**
- 有效的 EV 代码签名证书
- 公司 D-U-N-S 编号（可免费申请：https://www.dnb.com）
- 公司邮箱（@yourcompany.com）
- 公司法人信息

### 2️⃣ **创建驱动提交包**

使用 WDK 工具创建 `.cab` 提交包：

```cmd
REM 1. 编译驱动
msbuild IddSampleDriver.vcxproj /p:Configuration=Release /p:Platform=x64

REM 2. 用 EV 证书初步签名
signtool sign /v /fd sha256 /tr http://timestamp.digicert.com /td sha256 ^
  /sha1 [你的EV证书指纹] ^
  x64\Release\IddSampleDriver.dll

REM 3. 创建提交包
inf2cat /driver:x64\Release /os:10_X64
makecab x64\Release\IddSampleDriver.inf submission.ddf
```

#### **submission.ddf 示例：**
```
.OPTION EXPLICIT
.Set CabinetNameTemplate=IddSampleDriver.cab
.Set DiskDirectoryTemplate=CDROM
.Set CompressionType=MSZIP
.Set Cabinet=on
.Set Compress=on
x64\Release\IddSampleDriver.inf
x64\Release\IddSampleDriver.dll
x64\Release\IddSampleDriver.cat
```

### 3️⃣ **提交到 Microsoft**

1. **登录** Hardware Dev Center
2. **创建新提交** → Driver Submission
3. **上传 .cab 文件**
4. **选择认证类型**：
   - Windows 10/11 桌面（必选）
   - Windows Server（可选）
5. **填写驱动信息**：
   - 驱动名称
   - 版本号
   - 支持的 Windows 版本
   - 驱动类型（Display Driver）
6. **提交审核**

### 4️⃣ **Microsoft 审核流程**

| 阶段 | 时间 | 说明 |
|------|------|------|
| **Package Acceptance** | 1-2 小时 | 检查格式和基本要求 |
| **Manual Testing** | 3-5 个工作日 | Microsoft 手动测试 |
| **Automated Testing** | 1-2 天 | 自动化测试套件 |
| **Code Signing** | 几小时 | Microsoft 签名 |

### 5️⃣ **下载签名后的驱动**

审核通过后：
1. 登录 Hardware Dev Center
2. 找到你的提交
3. 下载 **Microsoft 签名**的驱动包
4. 解压后获得 `.cat` 文件（包含 Microsoft 签名）

---

## 测试签名（开发阶段）

在正式签名前，使用**测试签名**进行开发：

### 方法 A：使用 WDK 自动生成的测试证书

这是我们现在用的方法（需要测试模式）：

```cmd
REM 1. 启用测试模式
bcdedit /set testsigning on
shutdown /r /t 0

REM 2. 编译时 WDK 会自动生成测试证书
msbuild IddSampleDriver.vcxproj /p:Configuration=Release /p:Platform=x64
```

### 方法 B：手动创建测试证书

```cmd
REM 1. 创建测试证书
makecert -r -pe -ss PrivateCertStore ^
  -n "CN=IddSampleDriver Test Cert" ^
  -eku 1.3.6.1.5.5.7.3.3 ^
  IddTest.cer

REM 2. 安装到系统
certmgr /add IddTest.cer /s /r localMachine root
certmgr /add IddTest.cer /s /r localMachine trustedpublisher

REM 3. 签名驱动
signtool sign /v /s PrivateCertStore ^
  /n "IddSampleDriver Test Cert" ^
  /t http://timestamp.digicert.com ^
  x64\Release\IddSampleDriver.dll

REM 4. 生成 catalog
inf2cat /driver:x64\Release /os:10_X64

REM 5. 签名 catalog
signtool sign /v /s PrivateCertStore ^
  /n "IddSampleDriver Test Cert" ^
  /t http://timestamp.digicert.com ^
  x64\Release\IddSampleDriver.cat
```

---

## 费用对比

### 💰 **正统签名总费用**

| 项目 | 费用 (USD) | 周期 | 备注 |
|------|-----------|------|------|
| **EV 代码签名证书** | $299 - $799 | 1-3 年 | DigiCert 约 $474/年 |
| **Microsoft 硬件开发者账号** | $99 | 每年 | 公司账号必需 |
| **DUNS 编号** | 免费 | 一次性 | 可选，但建议申请 |
| **合计（首年）** | **$398 - $898** | - | 之后每年续费证书+$99 |

### 💡 **节省费用的方法**

1. **选择 Sectigo**：相比 DigiCert 便宜 $175/年
2. **多年购买**：EV 证书买 3 年通常有折扣
3. **代签名服务**：一些公司提供代签名服务（$100-300/次）
4. **开源项目**：Microsoft 有时为开源项目提供免费签名

---

## 签名验证

### 验证驱动是否正确签名：

```cmd
REM 查看签名信息
signtool verify /v /pa x64\Release\IddSampleDriver.dll

REM 查看 catalog 签名
signtool verify /v /pa x64\Release\IddSampleDriver.cat
```

### 正确签名的输出：

```
Verifying: IddSampleDriver.dll
Signature Index: 0 (Primary Signature)
Hash of file (sha256): XXXX...

Signing Certificate Chain:
    Issued to: Microsoft Code Signing PCA 2011
    Issued by: Microsoft Root Certificate Authority 2011
    ...

The signature is timestamped: Mon Nov 04 14:30:00 2025
Timestamp Verified by:
    ...

Successfully verified: IddSampleDriver.dll
```

---

## 推荐流程

### 🚀 **个人开发者/小团队：**

```
阶段 1: 开发（1-3 个月）
  └─ 使用测试签名 + 测试模式

阶段 2: 内测（1-2 周）
  └─ 继续使用测试签名，小范围测试

阶段 3: 准备发布
  ├─ 注册公司（如果是个人）
  ├─ 购买 EV 证书（Sectigo, $299/年）
  ├─ 注册 MS Hardware Dev Center（$99/年）
  └─ 申请 DUNS 编号（免费）

阶段 4: 正式签名
  ├─ 提交驱动到 Microsoft
  ├─ 等待审核（3-7 天）
  └─ 下载签名后的驱动

阶段 5: 发布
  └─ 用户无需测试模式即可安装
```

### 🏢 **企业开发者：**

```
建议直接使用正统签名流程：
1. 购买 DigiCert EV 证书（$474/年）
2. 注册 MS Hardware Dev Center
3. 每次发布都提交审核
4. 考虑购买多年证书节省费用
```

---

## 常见问题

### Q: 个人开发者能获得正统签名吗？
**A:** 可以，但需要：
1. 注册个体工商户或小型公司
2. 申请 DUNS 编号
3. 购买 EV 证书（以公司名义）

### Q: 能否跳过 Microsoft 认证？
**A:** ⚠️ **Windows 10 1607+** 要求所有内核模式驱动必须经过 Microsoft 认证。
- UMDF 用户模式驱动可以只用 EV 证书签名（不推荐）
- 但无法通过 Windows Update 分发

### Q: 测试签名的驱动能发布给客户吗？
**A:** ❌ **强烈不推荐**：
- 用户需要启用测试模式（降低安全性）
- 桌面会有"测试模式"水印
- 企业环境可能禁止测试模式

### Q: 审核一般需要多久？
**A:** 
- 正常情况：**3-5 个工作日**
- 复杂驱动：**1-2 周**
- 有问题需修改：**2-4 周**

### Q: 审核不通过怎么办？
**A:** Microsoft 会提供详细的失败日志：
1. 修复问题
2. 重新提交（无需额外费用）
3. 一般 2-3 次迭代能通过

---

## 有用的链接

- **Microsoft Hardware Dev Center**: https://partner.microsoft.com/dashboard/hardware
- **签名要求文档**: https://docs.microsoft.com/windows-hardware/drivers/dashboard/
- **EV 证书购买（DigiCert）**: https://www.digicert.com/signing/code-signing-certificates
- **EV 证书购买（Sectigo）**: https://www.sectigo.com/ssl-certificates-tls/code-signing
- **DUNS 编号申请**: https://www.dnb.com/duns-number.html
- **WDK 下载**: https://docs.microsoft.com/windows-hardware/drivers/download-the-wdk

---

## 总结

| 场景 | 建议方案 | 预算 |
|------|---------|------|
| **个人学习/原型开发** | 测试签名 + 测试模式 | **免费** |
| **小团队内测** | 测试签名 + 测试模式 | **免费** |
| **正式发布（个人）** | 注册公司 + Sectigo EV + MS 认证 | **~$400/年** |
| **企业产品** | DigiCert EV + MS 认证 | **~$573/年** |

对于现在的开发阶段，**继续使用测试模式是最佳选择**！✅



