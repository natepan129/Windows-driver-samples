@echo off
echo ========================================
echo Convert INF to UTF-16 LE encoding
echo ========================================
echo.

REM Convert IddSampleDriver_Fixed.inf to UTF-16 LE
powershell -Command "$content = Get-Content -Path 'IddSampleDriver_Fixed.inf' -Raw; $Utf16NoBomEncoding = New-Object System.Text.UnicodeEncoding $False, $True; [System.IO.File]::WriteAllText('IddSampleDriver.inf', $content, $Utf16NoBomEncoding)"

if %ERRORLEVEL% EQU 0 (
    echo [SUCCESS] IddSampleDriver.inf converted to UTF-16 LE
    echo.
    echo Checking file encoding:
    powershell -Command "$bytes = [System.IO.File]::ReadAllBytes('IddSampleDriver.inf')[0..3]; Write-Host 'First bytes:' ([System.BitConverter]::ToString($bytes))"
) else (
    echo [FAILED] Conversion failed
)

echo.
pause

