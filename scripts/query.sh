#!/usr/bin/env bash
set -euo pipefail

if [[ $# -eq 0 ]]; then
  echo "usage: scripts/query.sh stack <list|current|around>" >&2
  exit 1
fi

: "${XDG_RUNTIME_DIR:?XDG_RUNTIME_DIR is required}"
: "${HYPRLAND_INSTANCE_SIGNATURE:?HYPRLAND_INSTANCE_SIGNATURE is required}"

socket_path="$XDG_RUNTIME_DIR/hypr/$HYPRLAND_INSTANCE_SIGNATURE/.socket.sock"

printf 'hyprstack %s' "$*" | nc -U "$socket_path"
