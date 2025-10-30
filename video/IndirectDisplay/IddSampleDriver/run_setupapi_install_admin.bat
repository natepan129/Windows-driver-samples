@echo off
echo Running SetupAPI installation as Administrator...
powershell -ExecutionPolicy Bypass -Command "Start-Process powershell -Verb RunAs -ArgumentList '-ExecutionPolicy Bypass -NoExit -File install_with_setupapi.ps1'"


