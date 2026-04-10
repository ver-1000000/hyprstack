#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "$0")/.." && pwd)"

cd "$repo_root"

# Query-only smoke check.
# Hot reload is intentionally excluded because:
# - direct hyprctl unload/load crashed Hyprland on this machine
# - hyprpm is not yet wired into the local dev flow

echo '== stack list =='
scripts/query.sh stack list
echo
echo '== stack current =='
scripts/query.sh stack current
echo
echo '== stack around =='
scripts/query.sh stack around
