# Manual Cleanup Steps

## 🎯 Problem
Two devices in system:
- `ROOT\IDDSAMPLEDRIVER\0000` - Old, no Display Class
- `ROOT\IDDSAMPLEDRIVER\0001` - New, has Display Class ✅

## ✅ Method 1: Use Administrator PowerShell (Recommended)

```powershell
# 1. Right-click "Windows PowerShell" → "Run as administrator"

# 2. Delete old device
pnputil /remove-device "ROOT\IDDSAMPLEDRIVER\0000"

# 3. Verify
Get-PnpDevice | Where-Object { $_.InstanceId -like '*IddSampleDriver*' } | Format-Table FriendlyName, Status, Class, InstanceId -AutoSize
```

**Expected Result**: Only `ROOT\IDDSAMPLEDRIVER\0001` remains, with Display Class

---

## ✅ Method 2: Use Batch Script

```batch
# Right-click full_test_fixed_inf.bat → "Run as administrator"
```

This script will:
1. Clean all old devices
2. Reinstall (with fixed INF)
3. Verify results

---

## ✅ Method 3: Manual Delete in Device Manager

1. Open Device Manager (`devmgmt.msc`)
2. View → Show hidden devices
3. Find "IddSampleDriver Device"
4. Right-click → Uninstall device
5. ✓ Check "Delete the driver software for this device"
6. OK

---

## 🧪 Success Verification Criteria

Run this command:
```powershell
Get-PnpDevice | Where-Object { $_.InstanceId -like '*IddSampleDriver*' } | Format-Table FriendlyName, Status, Class, ClassGuid -AutoSize
```

**Expected Output**:
```
FriendlyName           Status Class   ClassGuid
IddSampleDriver Device OK     Display {4d36e968-e325-11ce-bfc1-08002be10318}
```

✅ Has `Class: Display`
✅ Has correct `ClassGuid`
✅ `Status: OK`




