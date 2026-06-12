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

## License

MIT
