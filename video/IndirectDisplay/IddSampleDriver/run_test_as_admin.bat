@echo off
:: Launch test script as Administrator
echo Launching test with Administrator privileges...
powershell -Command "Start-Process -FilePath '%~dp0test_gpt_fixes_admin.bat' -Verb RunAs"

