@echo off
REM VDD Control Tool Shortcut
REM Usage: vdd <command> [options]
REM Example: vdd init
REM          vdd status
REM          vdd install --inf x64\Release\IddSampleDriver\IddSampleDriver.inf

build\bin\Release\vddctl.exe %*









