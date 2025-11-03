# Patch vddsdk.cpp with debug output
Write-Host "=== Patching vddsdk.cpp with Debug Code ===" -ForegroundColor Cyan

# Read file
$content = Get-Content "vddsdk.cpp" -Raw

# Backup
Copy-Item "vddsdk.cpp" "vddsdk.cpp.before_patch" -Force
Write-Host "Backup created: vddsdk.cpp.before_patch" -ForegroundColor Green

# Apply patches
$patches = @(
    @{
        Old = '        // Stage INF to Driver Store
        BOOL needReboot = FALSE;
        if (!DiInstallDriverW(nullptr, absPath, DIIRFLAG_FORCE_INF, &needReboot)) {
            DWORD err = ::GetLastError();
            SetLastError("Failed to stage INF to Driver Store: " + std::to_string(err));
            return Status::DriverError;
        }'
        New = '        // Stage INF to Driver Store
        printf("[VDD] Step 1: Staging INF to Driver Store...\n");
        BOOL needReboot = FALSE;
        if (!DiInstallDriverW(nullptr, absPath, DIIRFLAG_FORCE_INF, &needReboot)) {
            DWORD err = ::GetLastError();
            printf("[VDD] FAILED at DiInstallDriverW, error = %lu\n", err);
            SetLastError("Failed to stage INF to Driver Store: " + std::to_string(err));
            return Status::DriverError;
        }
        printf("[VDD] SUCCESS: INF staged\n");'
    }
)

$patchCount = 0
foreach ($patch in $patches) {
    if ($content -match [regex]::Escape($patch.Old)) {
        $content = $content -replace [regex]::Escape($patch.Old), $patch.New
        $patchCount++
        Write-Host "Applied patch $patchCount" -ForegroundColor Green
    } else {
        Write-Host "Patch $patchCount NOT FOUND - skipping" -ForegroundColor Yellow
    }
}

# Write back
if ($patchCount -gt 0) {
    $content | Set-Content "vddsdk.cpp" -NoNewline
    Write-Host "`nPatched $patchCount locations" -ForegroundColor Green
    Write-Host "File updated successfully!" -ForegroundColor Green
} else {
    Write-Host "`nNO PATCHES APPLIED!" -ForegroundColor Red
    Write-Host "File may already be patched or format changed" -ForegroundColor Yellow
}

# Verify
$newContent = Get-Content "vddsdk.cpp" -Raw
if ($newContent -match '\[VDD\]') {
    Write-Host "`n✓ Verification: Debug code found in file" -ForegroundColor Green
} else {
    Write-Host "`n✗ Verification: Debug code NOT found in file" -ForegroundColor Red
}



# Patch vddsdk.cpp with debug output
Write-Host "=== Patching vddsdk.cpp with Debug Code ===" -ForegroundColor Cyan

# Read file
$content = Get-Content "vddsdk.cpp" -Raw

# Backup
Copy-Item "vddsdk.cpp" "vddsdk.cpp.before_patch" -Force
Write-Host "Backup created: vddsdk.cpp.before_patch" -ForegroundColor Green

# Apply patches
$patches = @(
    @{
        Old = '        // Stage INF to Driver Store
        BOOL needReboot = FALSE;
        if (!DiInstallDriverW(nullptr, absPath, DIIRFLAG_FORCE_INF, &needReboot)) {
            DWORD err = ::GetLastError();
            SetLastError("Failed to stage INF to Driver Store: " + std::to_string(err));
            return Status::DriverError;
        }'
        New = '        // Stage INF to Driver Store
        printf("[VDD] Step 1: Staging INF to Driver Store...\n");
        BOOL needReboot = FALSE;
        if (!DiInstallDriverW(nullptr, absPath, DIIRFLAG_FORCE_INF, &needReboot)) {
            DWORD err = ::GetLastError();
            printf("[VDD] FAILED at DiInstallDriverW, error = %lu\n", err);
            SetLastError("Failed to stage INF to Driver Store: " + std::to_string(err));
            return Status::DriverError;
        }
        printf("[VDD] SUCCESS: INF staged\n");'
    }
)

$patchCount = 0
foreach ($patch in $patches) {
    if ($content -match [regex]::Escape($patch.Old)) {
        $content = $content -replace [regex]::Escape($patch.Old), $patch.New
        $patchCount++
        Write-Host "Applied patch $patchCount" -ForegroundColor Green
    } else {
        Write-Host "Patch $patchCount NOT FOUND - skipping" -ForegroundColor Yellow
    }
}

# Write back
if ($patchCount -gt 0) {
    $content | Set-Content "vddsdk.cpp" -NoNewline
    Write-Host "`nPatched $patchCount locations" -ForegroundColor Green
    Write-Host "File updated successfully!" -ForegroundColor Green
} else {
    Write-Host "`nNO PATCHES APPLIED!" -ForegroundColor Red
    Write-Host "File may already be patched or format changed" -ForegroundColor Yellow
}

# Verify
$newContent = Get-Content "vddsdk.cpp" -Raw
if ($newContent -match '\[VDD\]') {
    Write-Host "`n✓ Verification: Debug code found in file" -ForegroundColor Green
} else {
    Write-Host "`n✗ Verification: Debug code NOT found in file" -ForegroundColor Red
}



