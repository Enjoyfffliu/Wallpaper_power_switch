# Wallpaper Power Switch — Qt Desktop App

**Date:** 2026-06-13
**Status:** Approved
**Target:** macOS 12+, Qt 6.x (C++17)

## Overview

将现有 shell 脚本包装为 Qt/C++ 桌面应用，Dock 常驻，提供 GUI 配置界面、实时状态显示、冲突检测。

## Project Structure

```
wallpaper-power-switch/
├── src/
│   ├── main.cpp
│   ├── MainWindow.h/cpp          # 主窗口，组装所有面板
│   ├── PowerMonitor.h/cpp        # IOKit 电源监听（实时 + QTimer 兜底）
│   ├── AppManager.h/cpp          # 引擎 App 生命周期
│   ├── WallpaperManager.h/cpp    # 壁纸获取/设置/备份/恢复
│   ├── AppValidator.h/cpp        # .app bundle 四步校验
│   └── ConflictResolver.h/cpp    # 冲突检测 + 弹窗
├── resources/
│   ├── app.qrc
│   └── icon.icns
├── CMakeLists.txt
├── install.sh                    # 保留
├── wallpaper-power-switch.sh     # 保留（CLI 用户）
└── README.md
```

## Window Layout

```
┌──────────────────────────────────────────────────┐
│  🔌 Wallpaper Power Switch         [—] [□] [×]  │
├────────────┬─────────────────────────────────────┤
│  状态面板   │  电源:  ● AC 充电中 (85%)          │
│            │  引擎:  WaifuX  ● 运行中            │
│            │  当前壁纸: [缩略图] path             │
│            │  备份壁纸: [缩略图] path             │
├────────────┼─────────────────────────────────────┤
│  设置面板   │  引擎选择: [combo box] [浏览]       │
│            │  ☑ 启动前验证 App                    │
│            │  恢复壁纸: [path edit] [浏览]        │
│            │  [更新为当前壁纸]                    │
│            │  检测间隔: [spinbox] 秒              │
│            │  [保存配置] [立即运行一次]           │
├────────────┴─────────────────────────────────────┤
│  ⚠ 冲突提示区域（按需出现）                      │
└──────────────────────────────────────────────────┘
```

## Core Modules

### PowerMonitor
- `IOKit` 注册电源回调，变化实时通知 → Qt signal
- `QTimer` 兜底轮询（间隔可配置）
- 返回: AC / Battery / Unknown（台式机默认 AC）

### AppManager
- 接收电源信号 → 启动/关闭引擎
- 调用 AppValidator 四步校验
- 启动: `NSWorkspace openApplicationAtURL:` 或 `open -a` via QProcess
- 关闭: AppleScript `tell app to quit` → `pkill -x` 强杀
- Signal: `appRunningChanged(bool)`

### AppValidator
- 检查 `.app` 目录存在
- 检查 `Contents/Info.plist` 存在
- 检查 `Contents/MacOS/` 目录存在
- 检查至少一个可执行文件（`+x`）
- 全部通过才允许启动

### WallpaperManager
- 获取当前壁纸: AppleScript `get picture of desktop 1`
- 设置所有桌面壁纸: AppleScript loop `set picture to POSIX file`
- 备份管理: 读写 `~/.cache/wallpaper-static-backup`
- Signal: `wallpaperChanged(QString)`

### ConflictResolver
- **场景 A**: 用户设静态壁纸 + 引擎运行中 → QMessageBox
  - [关闭引擎并设置] [取消] [强制设置]
- **场景 B**: 备份文件缺失/与当前不同 → UI 状态栏 ⚠ + [更新备份] 按钮

## Data Flow

```
PowerMonitor (IOKit/QTimer)
       │ signal: powerChanged(AC|Battery)
       ↓
   MainWindow
       │
       ├─→ AppManager → AppValidator → 启动/关闭引擎
       ├─→ WallpaperManager → AppleScript → 获取/设置壁纸
       └─→ ConflictResolver → QMessageBox 弹窗
```

## Configuration (QSettings)

文件: `~/Library/Preferences/com.flames.wallpaper-power-switch.plist`

| Key | Default | Description |
|-----|---------|-------------|
| `engineAppName` | `"WaifuX"` | 引擎 App 名称 |
| `engineAppPath` | `"/Applications/WaifuX.app"` | 引擎完整路径 |
| `restoreWallpaperPath` | `""` (auto) | 断电恢复壁纸路径 |
| `checkInterval` | `30` | 检测间隔（秒） |
| `validateBeforeLaunch` | `true` | 启动前校验 App |

## Packaging

- CMake `MACOSX_BUNDLE` target
- `macdeployqt` 打包 Qt frameworks
- 产物: `WallpaperPowerSwitch.app` (~60MB)
- DMG via `macdeployqt -dmg`

## Backward Compatibility

- 保留原 `wallpaper-power-switch.sh` 和 `install.sh`
- CLI 用户可继续使用 launchd + shell 方案
- GUI 应用使用独立的 QTimer 调度，不与 launchd plist 冲突
