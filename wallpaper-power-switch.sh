#!/bin/bash
#
# wallpaper-power-switch.sh
# 插电 → 启动动态壁纸 App
# 断电 → 关闭动态壁纸 App，恢复原生静态壁纸
#
# 每次运行都会确保 App 状态与电源状态一致（不只是状态变化时才动作）

set -euo pipefail

STATE_FILE="$HOME/.cache/wallpaper-power-state"
WAIFUX_APP="${WAIFUX_APP:-WaifuX}"

# ---------- 配置：改这里换成你想要的静态壁纸 ----------
# 留空则自动保存当前壁纸（首次运行时记录）
STATIC_WALLPAPER="${STATIC_WALLPAPER:-$HOME/.cache/wallpaper-static-backup}"
# ------------------------------------------------------

mkdir -p "$HOME/.cache"

log() { echo "[$(date '+%Y-%m-%d %H:%M:%S')] $*"; }

# --- 验证 App 是否有效（检查 .app bundle 完整性）---
validate_app() {
    local app_name="$1"
    local app_path="/Applications/${app_name}.app"

    [[ -d "$app_path" ]] || {
        log "ERROR: App bundle 不存在: $app_path"
        return 1
    }

    [[ -f "$app_path/Contents/Info.plist" ]] || {
        log "ERROR: App 缺少 Info.plist: $app_path"
        return 1
    }

    [[ -d "$app_path/Contents/MacOS" ]] || {
        log "ERROR: App 缺少可执行目录: $app_path/Contents/MacOS"
        return 1
    }

    # 至少有一个可执行文件
    local exe_count
    exe_count=$(find "$app_path/Contents/MacOS" -maxdepth 1 -type f -perm +111 2>/dev/null | wc -l)
    if [[ "$exe_count" -eq 0 ]]; then
        log "ERROR: App 中没有可执行文件: $app_path"
        return 1
    fi

    return 0
}

# --- 获取当前电源状态 ---
get_power_state() {
    # pmset -g batt 在 AC 电源时第一行包含 "AC Power"
    # 在电池时包含 "Battery Power"
    # 如果是台式机没有电池，pmset -g batt 会失败 → 默认当作 AC
    local line
    line=$(pmset -g batt 2>/dev/null | head -n 1)
    if [[ -z "$line" ]]; then
        # 没有电池信息（台式机），始终返回 AC
        echo "AC"
    elif echo "$line" | grep -q "AC Power"; then
        echo "AC"
    else
        echo "BATTERY"
    fi
}

# --- 获取当前壁纸路径（取 Desktop 1）---
get_current_wallpaper() {
    osascript -e 'tell application "System Events" to get picture of desktop 1' 2>/dev/null || true
}

# --- 设置所有桌面的壁纸 ---
set_all_wallpapers() {
    local img="$1"
    [[ -f "$img" ]] || {
        log "ERROR: 壁纸文件不存在: $img"
        return 1
    }
    osascript -e "
    tell application \"System Events\"
        set dc to count of desktops
        repeat with i from 1 to dc
            tell desktop i
                set picture to POSIX file \"$img\"
            end tell
        end repeat
    end tell" 2>&1
    local rc=$?
    if [[ $rc -ne 0 ]]; then
        log "ERROR: 设置壁纸失败 (exit=$rc): $img"
        return 1
    fi
    return 0
}

# --- WaifuX 是否在运行 ---
is_app_running() {
    pgrep -x "$WAIFUX_APP" >/dev/null 2>&1
}

# --- 启动动态壁纸 App ---
launch_app() {
    if ! validate_app "$WAIFUX_APP"; then
        log "FATAL: App 验证失败，无法启动 $WAIFUX_APP"
        return 1
    fi

    if is_app_running; then
        return 0  # 已经在运行
    fi

    open -a "$WAIFUX_APP" 2>&1
    local rc=$?
    if [[ $rc -ne 0 ]]; then
        log "ERROR: 启动 $WAIFUX_APP 失败 (exit=$rc)"
        return 1
    fi
    log "🔌 已插电 → 启动 $WAIFUX_APP"
    return 0
}

# --- 关闭动态壁纸 App ---
quit_app() {
    if ! is_app_running; then
        return 0  # 已经关了
    fi

    # 优雅退出
    osascript -e "tell application \"$WAIFUX_APP\" to quit" 2>&1 || true
    sleep 1

    # 如果还没死就强制杀
    if is_app_running; then
        pkill -x "$WAIFUX_APP" 2>&1 || true
        sleep 1
    fi

    if is_app_running; then
        log "ERROR: 无法关闭 $WAIFUX_APP"
        return 1
    fi

    log "🪫 已断电 → 关闭 $WAIFUX_APP"
    return 0
}

# --- 保存当前壁纸为静态备份 ---
save_static_wallpaper() {
    local current
    current=$(get_current_wallpaper)
    if [[ -n "$current" ]]; then
        echo "file://$current" > "$STATIC_WALLPAPER"
        log "📸 已保存当前壁纸: $current"
    else
        log "WARNING: 无法获取当前壁纸路径"
    fi
}

# --- 恢复静态壁纸 ---
restore_static_wallpaper() {
    if [[ ! -f "$STATIC_WALLPAPER" ]]; then
        log "WARNING: 未找到壁纸备份，尝试保存当前壁纸"
        save_static_wallpaper
        # 保存完就直接返回，因为当前就是静态壁纸
        return 0
    fi

    local img
    img=$(sed 's|^file://||' "$STATIC_WALLPAPER")
    if set_all_wallpapers "$img"; then
        log "🪫 已断电 → 恢复静态壁纸: $img"
    fi
}

# ==================== 主逻辑 ====================

POWER=$(get_power_state)
APP_VALID=false

# 先验证 App（不管当前是什么电源状态，先检查 App 是否可用）
if validate_app "$WAIFUX_APP"; then
    APP_VALID=true
fi

if [[ "$POWER" == "AC" ]]; then
    # ---- 插电：确保动态壁纸在运行 ----
    if $APP_VALID; then
        # 保存当前壁纸（如果还没保存过，或备份文件不存在）
        if [[ ! -f "$STATIC_WALLPAPER" ]]; then
            save_static_wallpaper
        fi
        launch_app
    fi

else
    # ---- 断电：确保动态壁纸已关闭，恢复静态壁纸 ----
    quit_app
    sleep 1
    restore_static_wallpaper
fi

# 记录当前状态（防止同一事件被重复处理，也留作调试）
echo "$POWER" > "$STATE_FILE"
