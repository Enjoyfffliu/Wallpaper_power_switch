#!/bin/bash
set -euo pipefail

echo "🖥️  Wallpaper Power Switch - 安装"

# 1. 安装脚本
mkdir -p "$HOME/.local/bin"
cp wallpaper-power-switch.sh "$HOME/.local/bin/wallpaper-power-switch.sh"
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
