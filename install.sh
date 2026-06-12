#!/bin/bash
set -euo pipefail

# 动态壁纸 App 名，可通过第一个参数指定，默认 WaifuX
APP_NAME="${1:-WaifuX}"

echo "🖥️  Wallpaper Power Switch - 安装"
echo "  动态壁纸 App: $APP_NAME"

# 1. 安装脚本（同时替换 App 名）
mkdir -p "$HOME/.local/bin"
sed "s/^WAIFUX_APP=.*/WAIFUX_APP=\"$APP_NAME\"/" wallpaper-power-switch.sh \
    > "$HOME/.local/bin/wallpaper-power-switch.sh"
chmod +x "$HOME/.local/bin/wallpaper-power-switch.sh"
echo "  ✓ 脚本已安装到 ~/.local/bin/"

# 2. 安装 launchd plist
sed "s|__HOME__|$HOME|" com.flames.wallpaper-power-switch.plist \
    > "$HOME/Library/LaunchAgents/com.flames.wallpaper-power-switch.plist"
echo "  ✓ LaunchAgent 已安装到 ~/Library/LaunchAgents/"

# 3. 加载并启动
launchctl unload "$HOME/Library/LaunchAgents/com.flames.wallpaper-power-switch.plist" 2>/dev/null || true
launchctl load "$HOME/Library/LaunchAgents/com.flames.wallpaper-power-switch.plist"
echo "  ✓ 服务已启动"

echo ""
echo "✅ 安装完成！"
echo ""
echo "现在插拔电源试试吧 🔌"
echo ""
echo "卸载方法："
echo "  launchctl unload ~/Library/LaunchAgents/com.flames.wallpaper-power-switch.plist"
echo "  rm ~/.local/bin/wallpaper-power-switch.sh"
echo "  rm ~/Library/LaunchAgents/com.flames.wallpaper-power-switch.plist"
