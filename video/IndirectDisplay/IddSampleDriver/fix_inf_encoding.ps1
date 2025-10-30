# Convert IddSampleDriver_Fixed.inf back to UTF-8 (no BOM)
$content = Get-Content -Path "IddSampleDriver_Fixed.inf" -Raw -Encoding Unicode
$Utf8NoBomEncoding = New-Object System.Text.UTF8Encoding $False
[System.IO.File]::WriteAllText("$PWD\IddSampleDriver_Fixed.inf", $content, $Utf8NoBomEncoding)

Write-Host "IddSampleDriver_Fixed.inf converted to UTF-8 (no BOM)" -ForegroundColor Green

