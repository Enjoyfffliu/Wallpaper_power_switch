#!/bin/bash
#
# wallpaper-power-switch.sh
# 插电 → 启动 WaifuX（动态壁纸）
# 断电 → 关闭 WaifuX，恢复原生静态壁纸
#

set -euo pipefail

STATE_FILE="$HOME/.cache/wallpaper-power-state"
WAIFUX_APP="WaifuX"

# ---------- 配置：改这里换成你想要的静态壁纸 ----------
# 留空则自动保存当前壁纸（首次运行时记录）
STATIC_WALLPAPER="${STATIC_WALLPAPER:-$HOME/.cache/wallpaper-static-backup}"
# ------------------------------------------------------

mkdir -p "$HOME/.cache"

# --- 获取当前电源状态 ---
get_power_state() {
    pmset -g batt 2>/dev/null | head -n 1 | grep -q "AC Power" && echo "AC" || echo "BATTERY"
}

# --- 获取当前壁纸路径（取 Desktop 1） ---
get_current_wallpaper() {
    osascript -e 'tell application "System Events" to get picture of desktop 1' 2>/dev/null || true
}

# --- 设置所有桌面的壁纸 ---
set_all_wallpapers() {
    local img="$1"
    [[ -f "$img" ]] || return 1
    osascript -e "
    tell application \"System Events\"
        set dc to count of desktops
        repeat with i from 1 to dc
            tell desktop i
                set picture to POSIX file \"$img\"
            end tell
        end repeat
    end tell" 2>/dev/null
}

# --- WaifuX 是否在运行 ---
is_waifux_running() {
    pgrep -x "$WAIFUX_APP" >/dev/null 2>&1
}

# --- 启动 WaifuX ---
launch_waifux() {
    if ! is_waifux_running; then
        open -a "$WAIFUX_APP"
    fi
}

# --- 关闭 WaifuX ---
quit_waifux() {
    if is_waifux_running; then
        osascript -e "tell application \"$WAIFUX_APP\" to quit" 2>/dev/null || true
        sleep 1
        # 如果还没死就强制
        pkill -x "$WAIFUX_APP" 2>/dev/null || true
    fi
}

# ==================== 主逻辑 ====================

POWER=$(get_power_state)
PREVIOUS=$(cat "$STATE_FILE" 2>/dev/null || echo "UNKNOWN")

# 状态没变，跳过
if [[ "$POWER" == "$PREVIOUS" ]]; then
    exit 0
fi

echo "$POWER" > "$STATE_FILE"

if [[ "$POWER" == "AC" ]]; then
    # ---- 插电：切动态壁纸 ----
    # 保存当前壁纸路径（如果还没保存过）
    if [[ ! -f "$STATIC_WALLPAPER" ]]; then
        CURRENT=$(get_current_wallpaper)
        echo "file://$CURRENT" > "$STATIC_WALLPAPER"
    fi
    launch_waifux
    echo "🔌 已插电 → 启动 WaifuX"

else
    # ---- 断电：恢复静态壁纸 ----
    quit_waifux
    sleep 1

    # 读取保存的壁纸路径
    if [[ -f "$STATIC_WALLPAPER" ]]; then
        IMG=$(cat "$STATIC_WALLPAPER" | sed 's|^file://||')
        set_all_wallpapers "$IMG" && echo "🪫 已断电 → 恢复静态壁纸: $IMG"
    else
        echo "🪫 已断电 → WaifuX 已关闭（未找到保存的壁纸，请手动设置）"
    fi
fi
