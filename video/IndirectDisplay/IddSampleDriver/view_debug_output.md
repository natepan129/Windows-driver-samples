# 查看驱动调试输出

## 方法 1：DebugView (推荐)

1. 下载 DebugView：
   https://learn.microsoft.com/en-us/sysinternals/downloads/debugview

2. 以管理员权限运行 DebugView

3. 菜单：Capture → Capture Kernel
   确保勾选 ✅ Capture Kernel

4. 重新安装驱动：
   ```powershell
   # 卸载旧驱动
   .\vddctl.exe uninstall
   
   # 重新安装
   .\vddctl.exe install "x64\Release\IddSampleDriver\IddSampleDriver.inf"
   ```

5. 在 DebugView 中搜索：`[IddSample]`

## 预期输出

如果成功，你应该看到：
```
[IddSample] AdapterInitFinished called, Status=0x0
[IddSample] Creating 3 virtual monitors...
[IddSample] Creating monitor 0...
[IddSample] Monitor 0 created successfully
[IddSample] Monitor 0 arrival reported successfully
[IddSample] Creating monitor 1...
[IddSample] Monitor 1 created successfully
[IddSample] Monitor 1 arrival reported successfully
[IddSample] Creating monitor 2...
[IddSample] Monitor 2 created successfully
[IddSample] Monitor 2 arrival reported successfully
[IddSample] All monitors created successfully
```

如果失败，会看到：
```
[IddSample] ERROR: Adapter initialization failed!
```
或
```
[IddSample] ERROR: Failed to create monitor X, Status=0xXXXXXXXX
```

## 方法 2：Event Viewer

1. 打开 Event Viewer (eventvwr.msc)
2. Windows Logs → System
3. 过滤：Source = "IddCx" 或搜索 "IndirectDisplay"
4. 查看是否有错误消息








