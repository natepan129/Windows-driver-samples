@echo off
REM 安裝 CMake 腳本
echo ========================================
echo 安裝 CMake
echo ========================================

echo [INFO] 檢查 CMake 是否已安裝...
cmake --version >nul 2>&1
if %errorLevel% == 0 (
    echo [SUCCESS] CMake 已安裝
    cmake --version
    goto :end
)

echo [INFO] CMake 未安裝，開始安裝...

REM 方法 1: 使用 Chocolatey (如果已安裝)
echo [INFO] 嘗試使用 Chocolatey 安裝 CMake...
choco install cmake -y >nul 2>&1
if %errorLevel% == 0 (
    echo [SUCCESS] 使用 Chocolatey 安裝 CMake 成功
    goto :end
)

REM 方法 2: 使用 winget (Windows 10/11)
echo [INFO] 嘗試使用 winget 安裝 CMake...
winget install Kitware.CMake >nul 2>&1
if %errorLevel% == 0 (
    echo [SUCCESS] 使用 winget 安裝 CMake 成功
    goto :end
)

REM 方法 3: 手動下載
echo [WARNING] 自動安裝失敗，請手動安裝 CMake
echo [INFO] 請前往 https://cmake.org/download/ 下載並安裝 CMake
echo [INFO] 安裝時請選擇 "Add CMake to the system PATH"

:end
echo.
echo ========================================
echo CMake 安裝完成
echo ========================================
pause
