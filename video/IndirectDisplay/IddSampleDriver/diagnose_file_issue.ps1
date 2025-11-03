# ============================================================================
# 快速诊断：检查文件保存问题的原因
# ============================================================================

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "   File Save Issue Diagnosis" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan

$targetFile = "vddsdk.cpp"
$targetDir = Get-Location

Write-Host "`nTarget File: $targetFile" -ForegroundColor Yellow
Write-Host "Target Dir: $targetDir" -ForegroundColor Yellow

# Test 1: 文件是否存在
Write-Host "`n[Test 1] File Existence" -ForegroundColor Cyan
if (Test-Path $targetFile) {
    Write-Host "✓ File exists" -ForegroundColor Green
    $fileInfo = Get-Item $targetFile
    Write-Host "  Size: $($fileInfo.Length) bytes" -ForegroundColor Gray
    Write-Host "  Modified: $($fileInfo.LastWriteTime)" -ForegroundColor Gray
} else {
    Write-Host "✗ File NOT found!" -ForegroundColor Red
    exit 1
}

# Test 2: 文件属性
Write-Host "`n[Test 2] File Attributes" -ForegroundColor Cyan
$attrib = attrib $targetFile
Write-Host "  $attrib" -ForegroundColor Gray
if ($fileInfo.IsReadOnly) {
    Write-Host "✗ File is READ-ONLY!" -ForegroundColor Red
} else {
    Write-Host "✓ Not read-only" -ForegroundColor Green
}

# Test 3: 写入测试
Write-Host "`n[Test 3] Write Test (same directory)" -ForegroundColor Cyan
try {
    "test" | Out-File "__write_test__.txt" -ErrorAction Stop
    Write-Host "✓ Can write to directory" -ForegroundColor Green
    Remove-Item "__write_test__.txt" -Force
} catch {
    Write-Host "✗ CANNOT write to directory!" -ForegroundColor Red
    Write-Host "  Error: $($_.Exception.Message)" -ForegroundColor Red
}

# Test 4: 权限检查
Write-Host "`n[Test 4] Permissions (ACL)" -ForegroundColor Cyan
$acl = Get-Acl $targetDir
Write-Host "  Owner: $($acl.Owner)" -ForegroundColor Gray
$currentUser = [System.Security.Principal.WindowsIdentity]::GetCurrent().Name
Write-Host "  Current User: $currentUser" -ForegroundColor Gray

$hasWriteAccess = $false
foreach ($access in $acl.Access) {
    if ($access.IdentityReference -like "*$env:USERNAME*" -or $access.IdentityReference -like "*Users*") {
        $rights = $access.FileSystemRights
        if ($rights -match "FullControl|Modify|Write") {
            $hasWriteAccess = $true
            Write-Host "  ✓ Has $rights permission" -ForegroundColor Green
        }
    }
}
if (-not $hasWriteAccess) {
    Write-Host "✗ NO WRITE PERMISSION!" -ForegroundColor Red
}

# Test 5: 进程锁定检查
Write-Host "`n[Test 5] Process Locks" -ForegroundColor Cyan
$lockingProcesses = Get-Process | Where-Object { 
    $_.ProcessName -like "*MSBuild*" -or 
    $_.ProcessName -like "*Code*" -or 
    $_.ProcessName -like "*devenv*" -or
    $_.ProcessName -like "*cl*"
}
if ($lockingProcesses) {
    Write-Host "⚠ Found processes that might lock files:" -ForegroundColor Yellow
    $lockingProcesses | Select-Object ProcessName, Id, StartTime | Format-Table -AutoSize
} else {
    Write-Host "✓ No obvious locking processes" -ForegroundColor Green
}

# Test 6: 路径长度
Write-Host "`n[Test 6] Path Length" -ForegroundColor Cyan
$fullPath = (Resolve-Path $targetFile).Path
$pathLength = $fullPath.Length
Write-Host "  Full path: $fullPath" -ForegroundColor Gray
Write-Host "  Length: $pathLength characters" -ForegroundColor Gray
if ($pathLength -gt 200) {
    Write-Host "⚠ Path is very long (>200 chars)" -ForegroundColor Yellow
} elseif ($pathLength -gt 150) {
    Write-Host "⚠ Path is moderately long (>150 chars)" -ForegroundColor Yellow
} else {
    Write-Host "✓ Path length is OK" -ForegroundColor Green
}

# Test 7: 磁盘空间
Write-Host "`n[Test 7] Disk Space" -ForegroundColor Cyan
$drive = (Get-Item $targetDir).PSDrive
$freeSpace = $drive.Free / 1GB
Write-Host "  Drive: $($drive.Name)" -ForegroundColor Gray
Write-Host "  Free Space: $([math]::Round($freeSpace, 2)) GB" -ForegroundColor Gray
if ($freeSpace -lt 1) {
    Write-Host "✗ Low disk space (<1GB)" -ForegroundColor Red
} else {
    Write-Host "✓ Disk space OK" -ForegroundColor Green
}

# Test 8: Windows Defender 受控资料夹存取
Write-Host "`n[Test 8] Controlled Folder Access (Windows Defender)" -ForegroundColor Cyan
try {
    $cfaStatus = Get-MpPreference | Select-Object -ExpandProperty EnableControlledFolderAccess -ErrorAction Stop
    if ($cfaStatus -eq 1) {
        Write-Host "⚠ Controlled Folder Access is ENABLED!" -ForegroundColor Yellow
        Write-Host "  This might block VS Code from writing files" -ForegroundColor Yellow
        Write-Host "  Solution: Add Code.exe to allowed apps or disable CFA" -ForegroundColor Yellow
    } else {
        Write-Host "✓ Controlled Folder Access is disabled" -ForegroundColor Green
    }
} catch {
    Write-Host "? Cannot check (requires Windows Defender)" -ForegroundColor Gray
}

# 总结
Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host "   Summary & Recommendations" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan

if ($pathLength -gt 150) {
    Write-Host "`n🔧 RECOMMENDATION: Move project to shorter path" -ForegroundColor Yellow
    Write-Host "   Current: $pathLength chars" -ForegroundColor Gray
    Write-Host "   Suggested: C:\IddSampleDriver" -ForegroundColor Green
}

if (-not $hasWriteAccess) {
    Write-Host "`n🔧 RECOMMENDATION: Fix permissions" -ForegroundColor Yellow
    Write-Host "   Run: icacls . /grant ${env:USERNAME}:(F) /T" -ForegroundColor Cyan
}

if ($lockingProcesses) {
    Write-Host "`n🔧 RECOMMENDATION: Close locking processes" -ForegroundColor Yellow
    Write-Host "   Or run: Get-Process MSBuild | Stop-Process -Force" -ForegroundColor Cyan
}

Write-Host "`n✨ BEST SOLUTION: Run fix_and_move_project.ps1" -ForegroundColor Green
Write-Host "   This will automatically fix all issues" -ForegroundColor Gray

Write-Host "`nPress Enter to exit..." -ForegroundColor Yellow
Read-Host


