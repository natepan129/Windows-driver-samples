# 直接修改 vddsdk.cpp 添加调试代码
$file = "vddsdk.cpp"
$content = Get-Content $file -Raw -Encoding UTF8

# 备份
Copy-Item $file "${file}.backup" -Force
Write-Host "已备份到 ${file}.backup" -ForegroundColor Green

# 在 InstallDriver 函数开头添加调试输出
$pattern = '(\s+Status InstallDriver\([^{]+\{)'
$replacement = '$1' + "`n    printf(`"[VDD] === InstallDriver START ===\n`");`n    printf(`"[VDD] INF Path: %ls\n`", infPath.c_str());"

$newContent = $content -replace $pattern, $replacement

# 保存
$newContent | Set-Content $file -Encoding UTF8 -NoNewline
Write-Host "已添加调试代码到 InstallDriver" -ForegroundColor Green

Write-Host "`n文件已修改，请重新编译测试" -ForegroundColor Cyan

