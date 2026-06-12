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

# --- 步骤1：列出已检测到的 App，弹原生选择框 ---
APP_PATH=""  # 最终选定的 .app 完整路径
if [[ ${#found[@]} -gt 0 ]]; then
    LIST=""
    for app in "${found[@]}"; do
        LIST+="\"$app\", "
    done
    LIST="${LIST%, }"

    PICKED=$(osascript -e "
    set appList to {$LIST}
    choose from list appList \
        with title \"选择动态壁纸 App\" \
        with prompt \"已检测到以下壁纸软件，请选择你要用的：\" \
        OK button name \"使用此 App\" \
        cancel button name \"浏览其他...\"
    " 2>/dev/null || true)

    if [[ -n "$PICKED" && "$PICKED" != "false" ]]; then
        APP_PATH="/Applications/${PICKED}.app"
    fi
fi

# --- 步骤2：没选到（没检测到 / 点了浏览其他）→ 弹出文件浏览器 ---
if [[ -z "$APP_PATH" ]]; then
    APP_PATH=$(osascript -e "
    set chosen to choose file \
        with prompt \"选择你的动态壁纸 App（通常在 /Applications 里）\" \
        default location (POSIX file \"/Applications\") \
        of type {\"com.apple.application-bundle\"} \
        invisibles false \
        showing package contents false
    return POSIX path of chosen
    " 2>/dev/null || true)
fi

# --- 校验 ---
if [[ -z "$APP_PATH" ]]; then
    echo "❌ 未选择任何 App，已取消安装"
    exit 1
fi

if [[ ! -d "$APP_PATH" ]]; then
    echo "❌ 找不到: $APP_PATH"
    exit 1
fi

# 从路径提取 App 名（去掉 .app 后缀）
APP_NAME=$(basename "$APP_PATH" .app)
echo ""
echo "📦 使用 App: $APP_NAME ($APP_PATH)"

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
