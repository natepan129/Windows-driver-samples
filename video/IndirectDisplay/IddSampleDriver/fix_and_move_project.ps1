# ============================================================================
# 一键修复：移动项目到短路径 + 修复权限 + 重新编译
# ============================================================================

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "   VDD SDK Project Fix & Move" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan

# 检查管理员权限
$isAdmin = ([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
if (-not $isAdmin) {
    Write-Host "`nERROR: This script requires Administrator privileges!" -ForegroundColor Red
    Write-Host "Right-click and select 'Run as Administrator'" -ForegroundColor Yellow
    pause
    exit 1
}

# 当前路径
$currentPath = Get-Location
Write-Host "`nCurrent Path: $currentPath" -ForegroundColor Yellow

# 新路径
$newPath = "C:\IddSampleDriver"
Write-Host "Target Path: $newPath" -ForegroundColor Green

# Step 1: 检查文件锁定
Write-Host "`n[Step 1] Checking file locks..." -ForegroundColor Cyan
$processes = Get-Process | Where-Object { 
    $_.ProcessName -like "*MSBuild*" -or 
    $_.ProcessName -like "*Code*" -or 
    $_.ProcessName -like "*devenv*" 
}
if ($processes) {
    Write-Host "Found processes that might lock files:" -ForegroundColor Yellow
    $processes | Select-Object ProcessName, Id | Format-Table -AutoSize
    Write-Host "Killing MSBuild processes..." -ForegroundColor Yellow
    Get-Process | Where-Object { $_.ProcessName -like "*MSBuild*" } | Stop-Process -Force -ErrorAction SilentlyContinue
    Write-Host "Done" -ForegroundColor Green
} else {
    Write-Host "No locking processes found" -ForegroundColor Green
}

# Step 2: 创建目标目录
Write-Host "`n[Step 2] Creating target directory..." -ForegroundColor Cyan
if (Test-Path $newPath) {
    Write-Host "WARNING: $newPath already exists!" -ForegroundColor Yellow
    $answer = Read-Host "Delete and recreate? (y/n)"
    if ($answer -eq 'y') {
        Remove-Item $newPath -Recurse -Force -ErrorAction SilentlyContinue
        Write-Host "Deleted" -ForegroundColor Green
    } else {
        Write-Host "Aborting..." -ForegroundColor Red
        pause
        exit 1
    }
}
New-Item -ItemType Directory -Path $newPath -Force | Out-Null
Write-Host "Created: $newPath" -ForegroundColor Green

# Step 3: 复制项目文件
Write-Host "`n[Step 3] Copying project files..." -ForegroundColor Cyan
Write-Host "This may take a few minutes..." -ForegroundColor Yellow

$robocopyArgs = @(
    $currentPath,
    $newPath,
    "/E",           # 复制所有子目录
    "/COPYALL",     # 复制所有属性
    "/R:1",         # 重试1次
    "/W:1",         # 等待1秒
    "/XD",          # 排除目录
    ".git",
    "build",
    ".vs",
    "x64\Debug",
    "/XF",          # 排除文件
    "*.pdb",
    "*.obj",
    "*.iobj",
    "*.ipdb",
    "/NFL",         # 不显示文件列表
    "/NDL",         # 不显示目录列表
    "/NJH",         # 不显示作业标题
    "/NJS",         # 不显示作业摘要
    "/NP"           # 不显示进度
)

$result = robocopy @robocopyArgs 2>&1

if ($LASTEXITCODE -le 7) {
    Write-Host "Copy completed successfully" -ForegroundColor Green
} else {
    Write-Host "Copy had errors (exit code: $LASTEXITCODE)" -ForegroundColor Red
    Write-Host "But continuing anyway..." -ForegroundColor Yellow
}

# Step 4: 修复权限
Write-Host "`n[Step 4] Fixing permissions..." -ForegroundColor Cyan
$currentUser = [System.Security.Principal.WindowsIdentity]::GetCurrent().Name
Write-Host "Granting full control to: $currentUser" -ForegroundColor Yellow
icacls $newPath /grant "${currentUser}:(F)" /T /Q | Out-Null
Write-Host "Permissions fixed" -ForegroundColor Green

# Step 5: 移除只读属性
Write-Host "`n[Step 5] Removing read-only attributes..." -ForegroundColor Cyan
Get-ChildItem $newPath -Recurse -File | Where-Object { $_.IsReadOnly } | ForEach-Object {
    $_.IsReadOnly = $false
}
Write-Host "Read-only attributes removed" -ForegroundColor Green

# Step 6: 启用长路径（可选）
Write-Host "`n[Step 6] Enabling long path support..." -ForegroundColor Cyan
try {
    $regPath = "HKLM:\SYSTEM\CurrentControlSet\Control\FileSystem"
    New-ItemProperty -Path $regPath -Name "LongPathsEnabled" -Value 1 -PropertyType DWORD -Force -ErrorAction Stop | Out-Null
    Write-Host "Long path support enabled (requires reboot)" -ForegroundColor Green
} catch {
    Write-Host "Already enabled or failed: $($_.Exception.Message)" -ForegroundColor Yellow
}

# Step 7: 验证关键文件
Write-Host "`n[Step 7] Verifying key files..." -ForegroundColor Cyan
$keyFiles = @(
    "vddsdk.cpp",
    "vddctl.cpp",
    "vddsdk.h",
    "CMakeLists.txt",
    "vddctl.vcxproj"
)

$allExist = $true
foreach ($file in $keyFiles) {
    $fullPath = Join-Path $newPath $file
    if (Test-Path $fullPath) {
        Write-Host "  ✓ $file" -ForegroundColor Green
    } else {
        Write-Host "  ✗ $file NOT FOUND!" -ForegroundColor Red
        $allExist = $false
    }
}

if (-not $allExist) {
    Write-Host "`nERROR: Some files are missing!" -ForegroundColor Red
    pause
    exit 1
}

# Step 8: 清理旧的构建文件
Write-Host "`n[Step 8] Cleaning old build artifacts..." -ForegroundColor Cyan
$buildPath = Join-Path $newPath "build"
if (Test-Path $buildPath) {
    Remove-Item $buildPath -Recurse -Force -ErrorAction SilentlyContinue
    Write-Host "Cleaned: $buildPath" -ForegroundColor Green
}

# 完成
Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host "   Fix & Move Completed!" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Cyan

Write-Host "`nNext Steps:" -ForegroundColor Yellow
Write-Host "1. Close this VS Code window" -ForegroundColor White
Write-Host "2. Open NEW folder in VS Code:" -ForegroundColor White
Write-Host "   $newPath" -ForegroundColor Cyan
Write-Host "3. Reopen vddsdk.cpp and save your changes" -ForegroundColor White
Write-Host "4. Rebuild and test" -ForegroundColor White

Write-Host "`nOld path (can be deleted later):" -ForegroundColor Yellow
Write-Host "  $currentPath" -ForegroundColor Gray

Write-Host "`nPress Enter to open new location in Explorer..." -ForegroundColor Yellow
Read-Host

# 打开新位置
explorer $newPath

Write-Host "Done!" -ForegroundColor Green



# ============================================================================
# 一键修复：移动项目到短路径 + 修复权限 + 重新编译
# ============================================================================

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "   VDD SDK Project Fix & Move" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan

# 检查管理员权限
$isAdmin = ([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
if (-not $isAdmin) {
    Write-Host "`nERROR: This script requires Administrator privileges!" -ForegroundColor Red
    Write-Host "Right-click and select 'Run as Administrator'" -ForegroundColor Yellow
    pause
    exit 1
}

# 当前路径
$currentPath = Get-Location
Write-Host "`nCurrent Path: $currentPath" -ForegroundColor Yellow

# 新路径
$newPath = "C:\IddSampleDriver"
Write-Host "Target Path: $newPath" -ForegroundColor Green

# Step 1: 检查文件锁定
Write-Host "`n[Step 1] Checking file locks..." -ForegroundColor Cyan
$processes = Get-Process | Where-Object { 
    $_.ProcessName -like "*MSBuild*" -or 
    $_.ProcessName -like "*Code*" -or 
    $_.ProcessName -like "*devenv*" 
}
if ($processes) {
    Write-Host "Found processes that might lock files:" -ForegroundColor Yellow
    $processes | Select-Object ProcessName, Id | Format-Table -AutoSize
    Write-Host "Killing MSBuild processes..." -ForegroundColor Yellow
    Get-Process | Where-Object { $_.ProcessName -like "*MSBuild*" } | Stop-Process -Force -ErrorAction SilentlyContinue
    Write-Host "Done" -ForegroundColor Green
} else {
    Write-Host "No locking processes found" -ForegroundColor Green
}

# Step 2: 创建目标目录
Write-Host "`n[Step 2] Creating target directory..." -ForegroundColor Cyan
if (Test-Path $newPath) {
    Write-Host "WARNING: $newPath already exists!" -ForegroundColor Yellow
    $answer = Read-Host "Delete and recreate? (y/n)"
    if ($answer -eq 'y') {
        Remove-Item $newPath -Recurse -Force -ErrorAction SilentlyContinue
        Write-Host "Deleted" -ForegroundColor Green
    } else {
        Write-Host "Aborting..." -ForegroundColor Red
        pause
        exit 1
    }
}
New-Item -ItemType Directory -Path $newPath -Force | Out-Null
Write-Host "Created: $newPath" -ForegroundColor Green

# Step 3: 复制项目文件
Write-Host "`n[Step 3] Copying project files..." -ForegroundColor Cyan
Write-Host "This may take a few minutes..." -ForegroundColor Yellow

$robocopyArgs = @(
    $currentPath,
    $newPath,
    "/E",           # 复制所有子目录
    "/COPYALL",     # 复制所有属性
    "/R:1",         # 重试1次
    "/W:1",         # 等待1秒
    "/XD",          # 排除目录
    ".git",
    "build",
    ".vs",
    "x64\Debug",
    "/XF",          # 排除文件
    "*.pdb",
    "*.obj",
    "*.iobj",
    "*.ipdb",
    "/NFL",         # 不显示文件列表
    "/NDL",         # 不显示目录列表
    "/NJH",         # 不显示作业标题
    "/NJS",         # 不显示作业摘要
    "/NP"           # 不显示进度
)

$result = robocopy @robocopyArgs 2>&1

if ($LASTEXITCODE -le 7) {
    Write-Host "Copy completed successfully" -ForegroundColor Green
} else {
    Write-Host "Copy had errors (exit code: $LASTEXITCODE)" -ForegroundColor Red
    Write-Host "But continuing anyway..." -ForegroundColor Yellow
}

# Step 4: 修复权限
Write-Host "`n[Step 4] Fixing permissions..." -ForegroundColor Cyan
$currentUser = [System.Security.Principal.WindowsIdentity]::GetCurrent().Name
Write-Host "Granting full control to: $currentUser" -ForegroundColor Yellow
icacls $newPath /grant "${currentUser}:(F)" /T /Q | Out-Null
Write-Host "Permissions fixed" -ForegroundColor Green

# Step 5: 移除只读属性
Write-Host "`n[Step 5] Removing read-only attributes..." -ForegroundColor Cyan
Get-ChildItem $newPath -Recurse -File | Where-Object { $_.IsReadOnly } | ForEach-Object {
    $_.IsReadOnly = $false
}
Write-Host "Read-only attributes removed" -ForegroundColor Green

# Step 6: 启用长路径（可选）
Write-Host "`n[Step 6] Enabling long path support..." -ForegroundColor Cyan
try {
    $regPath = "HKLM:\SYSTEM\CurrentControlSet\Control\FileSystem"
    New-ItemProperty -Path $regPath -Name "LongPathsEnabled" -Value 1 -PropertyType DWORD -Force -ErrorAction Stop | Out-Null
    Write-Host "Long path support enabled (requires reboot)" -ForegroundColor Green
} catch {
    Write-Host "Already enabled or failed: $($_.Exception.Message)" -ForegroundColor Yellow
}

# Step 7: 验证关键文件
Write-Host "`n[Step 7] Verifying key files..." -ForegroundColor Cyan
$keyFiles = @(
    "vddsdk.cpp",
    "vddctl.cpp",
    "vddsdk.h",
    "CMakeLists.txt",
    "vddctl.vcxproj"
)

$allExist = $true
foreach ($file in $keyFiles) {
    $fullPath = Join-Path $newPath $file
    if (Test-Path $fullPath) {
        Write-Host "  ✓ $file" -ForegroundColor Green
    } else {
        Write-Host "  ✗ $file NOT FOUND!" -ForegroundColor Red
        $allExist = $false
    }
}

if (-not $allExist) {
    Write-Host "`nERROR: Some files are missing!" -ForegroundColor Red
    pause
    exit 1
}

# Step 8: 清理旧的构建文件
Write-Host "`n[Step 8] Cleaning old build artifacts..." -ForegroundColor Cyan
$buildPath = Join-Path $newPath "build"
if (Test-Path $buildPath) {
    Remove-Item $buildPath -Recurse -Force -ErrorAction SilentlyContinue
    Write-Host "Cleaned: $buildPath" -ForegroundColor Green
}

# 完成
Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host "   Fix & Move Completed!" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Cyan

Write-Host "`nNext Steps:" -ForegroundColor Yellow
Write-Host "1. Close this VS Code window" -ForegroundColor White
Write-Host "2. Open NEW folder in VS Code:" -ForegroundColor White
Write-Host "   $newPath" -ForegroundColor Cyan
Write-Host "3. Reopen vddsdk.cpp and save your changes" -ForegroundColor White
Write-Host "4. Rebuild and test" -ForegroundColor White

Write-Host "`nOld path (can be deleted later):" -ForegroundColor Yellow
Write-Host "  $currentPath" -ForegroundColor Gray

Write-Host "`nPress Enter to open new location in Explorer..." -ForegroundColor Yellow
Read-Host

# 打开新位置
explorer $newPath

Write-Host "Done!" -ForegroundColor Green



