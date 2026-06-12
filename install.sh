#!/bin/bash
set -euo pipefail

# ============================================================
# Wallpaper Power Switch - 安装脚本
# 自动扫描已安装的动态壁纸 App，弹出原生对话框供选择
# ============================================================

echo "🖥️  Wallpaper Power Switch - 安装"
echo ""

# --- 扫描已知动态壁纸 App ---
KNOWN_APPS=("WaifuX" "Dynamic Wallpaper" "Dynamic Wallpaper Engine" "Live Wallpaper" "Plash" "Aerial" "Earth 3D" "Magic Window")

found=()
for app in "${KNOWN_APPS[@]}"; do
    if [[ -d "/Applications/${app}.app" ]]; then
        found+=("$app")
    fi
done

# --- 构造 AppleScript 选择列表 ---
if [[ ${#found[@]} -eq 0 ]]; then
    echo "📋 未检测到已知动态壁纸 App，请手动输入"
    read -p "👉 App 名称: " APP_NAME
else
    # 用原生对话框让用户选
    LIST=""
    for app in "${found[@]}"; do
        LIST+="\"$app\", "
    done
    LIST="${LIST%, }"  # 去掉末尾逗号

    APP_NAME=$(osascript -e "
    set appList to {$LIST}
    choose from list appList \
        with title \"选择动态壁纸 App\" \
        with prompt \"已检测到以下壁纸软件，请选择你要用的：\" \
        default items {item 1 of appList} \
        OK button name \"选择\" \
        cancel button name \"手动输入\"
    " 2>/dev/null || true)

    # 点了取消 → 手动输入
    if [[ -z "$APP_NAME" || "$APP_NAME" == "false" ]]; then
        echo ""
        read -p "👉 手动输入 App 名称: " APP_NAME
    fi
fi

echo ""
echo "📦 使用 App: $APP_NAME"

# --- 1. 安装脚本 ---
mkdir -p "$HOME/.local/bin"
sed "s/^WAIFUX_APP=.*/WAIFUX_APP=\"$APP_NAME\"/" wallpaper-power-switch.sh \
    > "$HOME/.local/bin/wallpaper-power-switch.sh"
chmod +x "$HOME/.local/bin/wallpaper-power-switch.sh"
echo "  ✓ 脚本已安装"

# --- 2. 安装 launchd plist ---
mkdir -p "$HOME/Library/LaunchAgents"
sed "s|__HOME__|$HOME|" com.flames.wallpaper-power-switch.plist \
    > "$HOME/Library/LaunchAgents/com.flames.wallpaper-power-switch.plist"
echo "  ✓ LaunchAgent 已安装"

# --- 3. 加载 ---
launchctl unload "$HOME/Library/LaunchAgents/com.flames.wallpaper-power-switch.plist" 2>/dev/null || true
launchctl load "$HOME/Library/LaunchAgents/com.flames.wallpaper-power-switch.plist"
echo "  ✓ 服务已启动"

echo ""
echo "✅ 安装完成！🔌 插拔电源试试吧"
echo ""
echo "卸载：launchctl unload ~/Library/LaunchAgents/com.flames.wallpaper-power-switch.plist && rm ~/.local/bin/wallpaper-power-switch.sh ~/Library/LaunchAgents/com.flames.wallpaper-power-switch.plist"
