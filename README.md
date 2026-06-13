# wallpaper-power-switch

> 🔌 插电动态壁纸，🔋 断电保留最后一帧 —— 根据电源状态自动切换壁纸。

## 它能干嘛

- **插上电源** → 自动启动你选择的动态壁纸 App
- **拔掉电源** → 自动关闭动态壁纸，壁纸保留最后一帧画面
- 开机自启，每 30 秒检测一次，只在状态变化时才动作

## 安装

```bash
git clone git@github.com:Enjoyfffliu/Wallpaper_power_switch.git
cd Wallpaper_power_switch
bash install.sh
```

安装脚本会自动扫描 `/Applications` 里的动态壁纸 App，弹出原生对话框让你选。

> 检测不到也没关系，可以点「浏览其他」弹出文件浏览器手动选择 .app 文件

## 卸载

```bash
launchctl unload ~/Library/LaunchAgents/com.flames.wallpaper-power-switch.plist
rm ~/.local/bin/wallpaper-power-switch.sh
rm ~/Library/LaunchAgents/com.flames.wallpaper-power-switch.plist
rm -rf ~/.cache/wallpaper-power-*
```

## 配置

已安装的文件位置：

| 文件 | 路径 |
|------|------|
| 切换脚本 | `~/.local/bin/wallpaper-power-switch.sh` |
| 定时触发器 | `~/Library/LaunchAgents/com.flames.wallpaper-power-switch.plist` |

### 换 App

```bash
# 编辑脚本，改第 11 行的 App 名
sed -i '' 's/WAIFUX_APP=".*"/WAIFUX_APP="你的App名"/' ~/.local/bin/wallpaper-power-switch.sh
```

### 改检测间隔

编辑 `~/Library/LaunchAgents/com.flames.wallpaper-power-switch.plist`，把第 13 行的 `30` 改成你想要的秒数，然后重新加载：

```bash
launchctl unload ~/Library/LaunchAgents/com.flames.wallpaper-power-switch.plist
launchctl load ~/Library/LaunchAgents/com.flames.wallpaper-power-switch.plist
```

## 原理

```
┌──────────────┐    每30秒     ┌──────────────────┐
│  launchd     │ ──────────→  │  检测电源状态     │
│  定时触发器   │              │  pmset -g batt    │
└──────────────┘              └────────┬─────────┘
                                       │
                               状态变了？
                              ╱         ╲
                            YES          NO
                           ╱               ╲
                     ┌─────┴─────┐      直接跳过
                     │  AC 电源  │
                     └─────┬─────┘
                           │
                 open -a <动态壁纸 App>

                     ┌─────┴─────┐
                     │  电池供电  │
                     └─────┬─────┘
                           │
                  关闭动态壁纸 App
                （桌面保留最后一帧画面）
```

## 兼容性

- macOS 12+ (Monterey 及以上)
- Apple Silicon / Intel 均支持
- 依赖：仅 macOS 自带工具（`pmset`、`osascript`、`launchd`、`open`），零额外依赖

## Qt 桌面应用

本项目也包含一个 Qt/C++ 桌面应用，提供 GUI 配置界面。

### 用户下载

从 [Releases](https://github.com/Enjoyfffliu/Wallpaper_power_switch/releases) 下载 `WallpaperPowerSwitch.app.zip`，解压双击运行即可。**用户无需安装 Qt 或任何依赖。**

### 开发者构建

```bash
# 安装依赖（仅构建者需要）
brew install cmake qt@6

# 克隆并构建
git clone git@github.com:Enjoyfffliu/Wallpaper_power_switch.git
cd Wallpaper_power_switch
mkdir build-release && cd build-release
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=/opt/homebrew/lib/cmake
make -j4

# 打包 Qt 框架到 .app
macdeployqt WallpaperPowerSwitch.app

# 修复 rpath（防止同时加载两个 Qt 副本）
APP="WallpaperPowerSwitch.app"
BIN="$APP/Contents/MacOS/WallpaperPowerSwitch"
install_name_tool -delete_rpath /opt/homebrew/lib "$BIN"
install_name_tool -add_rpath @executable_path/../Frameworks "$BIN"
for lib in QtCore QtGui QtWidgets QtDBus QtNetwork QtOpenGL; do
    install_name_tool -change \
        "/opt/homebrew/opt/qtbase/lib/${lib}.framework/Versions/A/${lib}" \
        "@rpath/${lib}.framework/Versions/A/${lib}" "$BIN"
done

# 打开
open WallpaperPowerSwitch.app
```

### GUI 功能

- 实时显示电源状态、引擎运行状态、当前壁纸缩略图
- GUI 选择壁纸引擎 App（combo box + 文件浏览）
- 配置检测间隔（5-300 秒）
- 断电恢复壁纸的浏览/更新/设置
- 冲突检测弹窗：静态壁纸 vs 引擎冲突 / 备份过期
- 配置持久化（QSettings）

### CLI vs GUI

| | CLI (shell) | GUI (Qt) |
|---|---|---|
| 启动方式 | launchd 后台 | Dock 应用 |
| 配置 | 编辑脚本/plist | GUI 界面 |
| 状态可见 | 日志文件 | 实时面板 + 缩略图 |
| 用户依赖 | 无 | 无（.app 自包含） |
| 体积 | ~5KB | ~84MB（含 Qt 框架） |

## License

MIT
