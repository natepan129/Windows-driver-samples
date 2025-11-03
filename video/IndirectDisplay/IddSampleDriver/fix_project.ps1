# ============================================================================
# Quick Fix: Move project to short path and fix permissions
# ============================================================================

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "   VDD SDK Project Fix" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan

# Check admin
$isAdmin = ([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
if (-not $isAdmin) {
    Write-Host ""
    Write-Host "ERROR: Need Administrator privileges!" -ForegroundColor Red
    Write-Host "Right-click and select 'Run as Administrator'" -ForegroundColor Yellow
    pause
    exit 1
}

# Paths
$currentPath = Get-Location
$newPath = "C:\IddSampleDriver"

Write-Host ""
Write-Host "Current: $currentPath" -ForegroundColor Yellow
Write-Host "Target: $newPath" -ForegroundColor Green

# Step 1: Kill processes
Write-Host ""
Write-Host "[1] Stopping MSBuild processes..." -ForegroundColor Cyan
Get-Process | Where-Object { $_.ProcessName -like "*MSBuild*" } | Stop-Process -Force -ErrorAction SilentlyContinue
Write-Host "Done" -ForegroundColor Green

# Step 2: Create directory
Write-Host ""
Write-Host "[2] Creating target directory..." -ForegroundColor Cyan
if (Test-Path $newPath) {
    Write-Host "WARNING: Directory exists!" -ForegroundColor Yellow
    $answer = Read-Host "Delete and recreate? (y/n)"
    if ($answer -ne 'y') {
        Write-Host "Aborting..." -ForegroundColor Red
        pause
        exit 1
    }
    Remove-Item $newPath -Recurse -Force -ErrorAction SilentlyContinue
}
New-Item -ItemType Directory -Path $newPath -Force | Out-Null
Write-Host "Created" -ForegroundColor Green

# Step 3: Copy files
Write-Host ""
Write-Host "[3] Copying files..." -ForegroundColor Cyan
Write-Host "Please wait..." -ForegroundColor Yellow

robocopy $currentPath $newPath /E /COPYALL /R:1 /W:1 /XD .git build .vs x64\Debug /XF *.pdb *.obj *.iobj *.ipdb /NFL /NDL /NJH /NJS /NP | Out-Null

if ($LASTEXITCODE -le 7) {
    Write-Host "Copy completed" -ForegroundColor Green
} else {
    Write-Host "Copy had errors but continuing..." -ForegroundColor Yellow
}

# Step 4: Fix permissions
Write-Host ""
Write-Host "[4] Fixing permissions..." -ForegroundColor Cyan
$currentUser = [System.Security.Principal.WindowsIdentity]::GetCurrent().Name
icacls $newPath /grant "${currentUser}:(F)" /T /Q | Out-Null
Write-Host "Permissions fixed" -ForegroundColor Green

# Step 5: Remove read-only
Write-Host ""
Write-Host "[5] Removing read-only attributes..." -ForegroundColor Cyan
Get-ChildItem $newPath -Recurse -File | Where-Object { $_.IsReadOnly } | ForEach-Object {
    $_.IsReadOnly = $false
}
Write-Host "Done" -ForegroundColor Green

# Step 6: Enable long paths
Write-Host ""
Write-Host "[6] Enabling long path support..." -ForegroundColor Cyan
try {
    $regPath = "HKLM:\SYSTEM\CurrentControlSet\Control\FileSystem"
    New-ItemProperty -Path $regPath -Name "LongPathsEnabled" -Value 1 -PropertyType DWORD -Force -ErrorAction Stop | Out-Null
    Write-Host "Enabled (reboot required)" -ForegroundColor Green
} catch {
    Write-Host "Already enabled or failed" -ForegroundColor Yellow
}

# Step 7: Verify files
Write-Host ""
Write-Host "[7] Verifying key files..." -ForegroundColor Cyan
$keyFiles = @("vddsdk.cpp", "vddctl.cpp", "vddsdk.h", "CMakeLists.txt")
$allExist = $true
foreach ($file in $keyFiles) {
    $fullPath = Join-Path $newPath $file
    if (Test-Path $fullPath) {
        Write-Host "  OK: $file" -ForegroundColor Green
    } else {
        Write-Host "  MISSING: $file" -ForegroundColor Red
        $allExist = $false
    }
}

if (-not $allExist) {
    Write-Host ""
    Write-Host "ERROR: Some files are missing!" -ForegroundColor Red
    pause
    exit 1
}

# Done
Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "   Completed Successfully!" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Cyan

Write-Host ""
Write-Host "Next Steps:" -ForegroundColor Yellow
Write-Host "1. Close VS Code / Cursor" -ForegroundColor White
Write-Host "2. Open folder: $newPath" -ForegroundColor Cyan
Write-Host "3. Edit and save vddsdk.cpp" -ForegroundColor White
Write-Host "4. Rebuild and test" -ForegroundColor White

Write-Host ""
Write-Host "Old path (can delete later):" -ForegroundColor Gray
Write-Host "  $currentPath" -ForegroundColor DarkGray

Write-Host ""
Write-Host "Press Enter to open new location..." -ForegroundColor Yellow
Read-Host

explorer $newPath

Write-Host "Done!" -ForegroundColor Green



# ============================================================================
# Quick Fix: Move project to short path and fix permissions
# ============================================================================

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "   VDD SDK Project Fix" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan

# Check admin
$isAdmin = ([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
if (-not $isAdmin) {
    Write-Host ""
    Write-Host "ERROR: Need Administrator privileges!" -ForegroundColor Red
    Write-Host "Right-click and select 'Run as Administrator'" -ForegroundColor Yellow
    pause
    exit 1
}

# Paths
$currentPath = Get-Location
$newPath = "C:\IddSampleDriver"

Write-Host ""
Write-Host "Current: $currentPath" -ForegroundColor Yellow
Write-Host "Target: $newPath" -ForegroundColor Green

# Step 1: Kill processes
Write-Host ""
Write-Host "[1] Stopping MSBuild processes..." -ForegroundColor Cyan
Get-Process | Where-Object { $_.ProcessName -like "*MSBuild*" } | Stop-Process -Force -ErrorAction SilentlyContinue
Write-Host "Done" -ForegroundColor Green

# Step 2: Create directory
Write-Host ""
Write-Host "[2] Creating target directory..." -ForegroundColor Cyan
if (Test-Path $newPath) {
    Write-Host "WARNING: Directory exists!" -ForegroundColor Yellow
    $answer = Read-Host "Delete and recreate? (y/n)"
    if ($answer -ne 'y') {
        Write-Host "Aborting..." -ForegroundColor Red
        pause
        exit 1
    }
    Remove-Item $newPath -Recurse -Force -ErrorAction SilentlyContinue
}
New-Item -ItemType Directory -Path $newPath -Force | Out-Null
Write-Host "Created" -ForegroundColor Green

# Step 3: Copy files
Write-Host ""
Write-Host "[3] Copying files..." -ForegroundColor Cyan
Write-Host "Please wait..." -ForegroundColor Yellow

robocopy $currentPath $newPath /E /COPYALL /R:1 /W:1 /XD .git build .vs x64\Debug /XF *.pdb *.obj *.iobj *.ipdb /NFL /NDL /NJH /NJS /NP | Out-Null

if ($LASTEXITCODE -le 7) {
    Write-Host "Copy completed" -ForegroundColor Green
} else {
    Write-Host "Copy had errors but continuing..." -ForegroundColor Yellow
}

# Step 4: Fix permissions
Write-Host ""
Write-Host "[4] Fixing permissions..." -ForegroundColor Cyan
$currentUser = [System.Security.Principal.WindowsIdentity]::GetCurrent().Name
icacls $newPath /grant "${currentUser}:(F)" /T /Q | Out-Null
Write-Host "Permissions fixed" -ForegroundColor Green

# Step 5: Remove read-only
Write-Host ""
Write-Host "[5] Removing read-only attributes..." -ForegroundColor Cyan
Get-ChildItem $newPath -Recurse -File | Where-Object { $_.IsReadOnly } | ForEach-Object {
    $_.IsReadOnly = $false
}
Write-Host "Done" -ForegroundColor Green

# Step 6: Enable long paths
Write-Host ""
Write-Host "[6] Enabling long path support..." -ForegroundColor Cyan
try {
    $regPath = "HKLM:\SYSTEM\CurrentControlSet\Control\FileSystem"
    New-ItemProperty -Path $regPath -Name "LongPathsEnabled" -Value 1 -PropertyType DWORD -Force -ErrorAction Stop | Out-Null
    Write-Host "Enabled (reboot required)" -ForegroundColor Green
} catch {
    Write-Host "Already enabled or failed" -ForegroundColor Yellow
}

# Step 7: Verify files
Write-Host ""
Write-Host "[7] Verifying key files..." -ForegroundColor Cyan
$keyFiles = @("vddsdk.cpp", "vddctl.cpp", "vddsdk.h", "CMakeLists.txt")
$allExist = $true
foreach ($file in $keyFiles) {
    $fullPath = Join-Path $newPath $file
    if (Test-Path $fullPath) {
        Write-Host "  OK: $file" -ForegroundColor Green
    } else {
        Write-Host "  MISSING: $file" -ForegroundColor Red
        $allExist = $false
    }
}

if (-not $allExist) {
    Write-Host ""
    Write-Host "ERROR: Some files are missing!" -ForegroundColor Red
    pause
    exit 1
}

# Done
Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "   Completed Successfully!" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Cyan

Write-Host ""
Write-Host "Next Steps:" -ForegroundColor Yellow
Write-Host "1. Close VS Code / Cursor" -ForegroundColor White
Write-Host "2. Open folder: $newPath" -ForegroundColor Cyan
Write-Host "3. Edit and save vddsdk.cpp" -ForegroundColor White
Write-Host "4. Rebuild and test" -ForegroundColor White

Write-Host ""
Write-Host "Old path (can delete later):" -ForegroundColor Gray
Write-Host "  $currentPath" -ForegroundColor DarkGray

Write-Host ""
Write-Host "Press Enter to open new location..." -ForegroundColor Yellow
Read-Host

explorer $newPath

Write-Host "Done!" -ForegroundColor Green



