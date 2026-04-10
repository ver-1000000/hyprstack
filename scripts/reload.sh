#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "$0")/.." && pwd)"
plugin_path="$repo_root/hyprstack-dev.so"

cd "$repo_root"

make all
make dev-plugin

hyprctl plugin unload "$plugin_path" >/dev/null 2>&1 || true
hyprctl plugin load "$plugin_path"
