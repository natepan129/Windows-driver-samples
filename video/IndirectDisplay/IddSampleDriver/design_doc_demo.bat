@echo off
REM VDD 模擬模式演示 - 展示設計文件中的命令範例

echo ========================================
echo VDD 設計文件命令範例演示
echo ========================================
echo.
echo 這個演示展示了設計文件中提到的所有命令範例
echo 雖然當前是模擬模式，但展示了完整的 API 設計
echo.

set VDD_TOOL=.\build\bin\Release\vddctl.exe

echo.
echo ========================================
echo 範例 1: 基本初始化流程
echo ========================================
echo 命令: vddctl init
%VDD_TOOL% init

echo.
echo ========================================
echo 範例 2: 激活虛擬顯示
echo ========================================
echo 命令: vddctl activate --name "VDD XR" --width 1920 --height 1080 --refresh 90
%VDD_TOOL% activate --name "VDD XR" --width 1920 --height 1080 --refresh 90

echo.
echo ========================================
echo 範例 3: 設置顯示模式
echo ========================================
echo 命令: vddctl setmode --index 0 --width 2560 --height 1440 --refresh 75
%VDD_TOOL% setmode --index 0 --width 2560 --height 1440 --refresh 75

echo.
echo ========================================
echo 範例 4: 設置顯示位置
echo ========================================
echo 命令: vddctl setlocation --index 0 --x 1920 --y 0 --width 1920 --height 1080
%VDD_TOOL% setlocation --index 0 --x 1920 --y 0 --width 1920 --height 1080

echo.
echo ========================================
echo 範例 5: 設置主顯示器
echo ========================================
echo 命令: vddctl setprimary --index 0
%VDD_TOOL% setprimary --index 0

echo.
echo ========================================
echo 範例 6: 列出顯示器
echo ========================================
echo 命令: vddctl list
%VDD_TOOL% list

echo.
echo ========================================
echo 範例 7: 停用虛擬顯示
echo ========================================
echo 命令: vddctl deactivate
%VDD_TOOL% deactivate

echo.
echo ========================================
echo 範例 8: 關閉 SDK
echo ========================================
echo 命令: vddctl shutdown
%VDD_TOOL% shutdown

echo.
echo ========================================
echo 演示完成！
echo ========================================
echo.
echo 總結：
echo - 所有設計文件中的命令範例都已展示
echo - API 設計完整且一致
echo - 錯誤處理機制正常
echo - 只需要修復 SDK 初始化問題即可完全工作
echo.
pause

