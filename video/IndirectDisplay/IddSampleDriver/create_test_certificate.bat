@echo off
REM 创建测试签名证书用于驱动开发

echo ========================================
echo 创建测试签名证书
echo ========================================
echo.

REM 1. 创建自签名证书
makecert -r -pe -ss PrivateCertStore -n "CN=IddSampleDriver Test Certificate" -eku 1.3.6.1.5.5.7.3.3 IddSampleDriverTest.cer

REM 2. 安装证书到受信任的根证书颁发机构
certmgr /add IddSampleDriverTest.cer /s /r localMachine root

REM 3. 安装证书到受信任的发布者
certmgr /add IddSampleDriverTest.cer /s /r localMachine trustedpublisher

echo.
echo ========================================
echo 证书创建完成！
echo 现在需要启用测试模式：
echo   bcdedit /set testsigning on
echo   重启电脑
echo ========================================

pause




