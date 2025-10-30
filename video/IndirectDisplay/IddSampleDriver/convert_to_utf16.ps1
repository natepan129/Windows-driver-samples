# Convert IddSampleDriver_Fixed.inf to UTF-16 LE
$content = Get-Content -Path "IddSampleDriver_Fixed.inf" -Raw
$Utf16NoBomEncoding = New-Object System.Text.UnicodeEncoding $False, $True
[System.IO.File]::WriteAllText("$PWD\IddSampleDriver.inf", $content, $Utf16NoBomEncoding)

Write-Host "IddSampleDriver.inf converted to UTF-16 LE" -ForegroundColor Green

