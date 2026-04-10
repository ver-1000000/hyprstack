#!/usr/bin/env bash
set -euo pipefail

if [[ $# -eq 0 ]]; then
  echo "usage: scripts/query.sh stack <list|current|around>" >&2
  exit 1
fi

: "${XDG_RUNTIME_DIR:?XDG_RUNTIME_DIR is required}"

resolve_signature() {
  local signatures
  mapfile -t signatures < <(
    find "$XDG_RUNTIME_DIR/hypr" -maxdepth 2 -type s -name '.socket.sock' 2>/dev/null \
      | sed 's#/.socket.sock$##' \
      | xargs -r -n1 basename \
      | sort -r
  )

  if [[ -n "${HYPRLAND_INSTANCE_SIGNATURE:-}" ]]; then
    signatures=("$HYPRLAND_INSTANCE_SIGNATURE" "${signatures[@]}")
  fi

  local seen=()
  local signature
  for signature in "${signatures[@]}"; do
    [[ -z "$signature" ]] && continue
    [[ " ${seen[*]} " == *" $signature "* ]] && continue
    seen+=("$signature")

    local socket="$XDG_RUNTIME_DIR/hypr/$signature/.socket.sock"
    [[ -S "$socket" ]] || continue

    local probe
    probe="$(printf 'hyprstack' | nc -U "$socket" 2>/dev/null || true)"

    if [[ -n "$probe" && "$probe" != "unknown request" ]]; then
      printf '%s\n' "$signature"
      return 0
    fi
  done

  return 1
}

signature="$(resolve_signature)"

if [[ -z "$signature" ]]; then
  echo "could not resolve a live Hyprland instance signature" >&2
  exit 1
fi

socket_path="$XDG_RUNTIME_DIR/hypr/$signature/.socket.sock"

printf 'hyprstack %s' "$*" | nc -U "$socket_path"
