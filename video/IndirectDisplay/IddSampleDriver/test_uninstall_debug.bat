@echo off
cd /d "%~dp0"
build\bin\Release\vddctl.exe uninstall > uninstall_output.txt 2>&1
type uninstall_output.txt
pause

