#!/usr/bin/env bash

set -euo pipefail

if [[ $# -eq 0 ]]; then
    echo "usage: $0 <hyprctl args...>" >&2
    exit 1
fi

runtime_dir="${XDG_RUNTIME_DIR:-/run/user/$(id -u)}"
hypr_dir="$runtime_dir/hypr"

if [[ ! -d "$hypr_dir" ]]; then
    echo "hypr runtime directory not found: $hypr_dir" >&2
    exit 1
fi

live_instance=""

while IFS= read -r sock; do
    if printf 'j/monitors' | timeout 1 nc -U "$sock" >/dev/null 2>&1; then
        live_instance="$(basename "$(dirname "$sock")")"
    fi
done < <(find "$hypr_dir" -maxdepth 2 -type s -name '.socket.sock' | sort)

if [[ -z "$live_instance" ]]; then
    echo "no live Hyprland instance found under $hypr_dir" >&2
    exit 1
fi

HYPRLAND_INSTANCE_SIGNATURE="$live_instance" exec hyprctl "$@"
