# wallpaper-power-switch

> 🔌 插电动态壁纸，🔋 断电静态壁纸 —— macOS 电源状态感知的壁纸自动切换工具。

## 它能干嘛

- **插上电源** → 自动启动 WaifuX（或你指定的任何动态壁纸 App）
- **拔掉电源** → 自动关闭动态壁纸、恢复系统原生静态壁纸
- 全自动，开机自启，每 30 秒检测一次，只在状态变化时才动作

## 安装

```bash
git clone https://github.com/你的用户名/wallpaper-power-switch.git
cd wallpaper-power-switch
bash install.sh
```

## 卸载

```bash
launchctl unload ~/Library/LaunchAgents/com.flames.wallpaper-power-switch.plist
rm ~/.local/bin/wallpaper-power-switch.sh
rm ~/Library/LaunchAgents/com.flames.wallpaper-power-switch.plist
rm -rf ~/.cache/wallpaper-power-*
```

## 配置

编辑 `~/.local/bin/wallpaper-power-switch.sh`：

| 变量 | 说明 | 默认值 |
|------|------|--------|
| `WAIFUX_APP` (第11行) | 动态壁纸 App 名称 | `WaifuX` |
| `StartInterval` (plist第13行) | 检测间隔（秒） | `30` |

### 换成其他动态壁纸 App

```bash
# 比如换成 Dynamic Wallpaper Engine
sed -i '' 's/WaifuX/你的App名/' ~/.local/bin/wallpaper-power-switch.sh
```

修改 plist 间隔后需重新加载：

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
                     open -a WaifuX
                     
                     ┌─────┴─────┐
                     │  电池供电  │
                     └─────┬─────┘
                           │
                     kill WaifuX
                     + 恢复静态壁纸
```

## 兼容性

- macOS 12+ (Monterey 及以上)
- Apple Silicon / Intel 均支持
- 依赖：仅系统自带工具（`pmset`、`osascript`、`launchd`），无需 brew 装任何东西

## License

MIT
