# hyprstack

`hyprstack` is a Hyprland plugin that adds a stable per-workspace window stack.

The goal of `hyprstack` is to make window order a first-class concept.

Hyprland already has multiple ordering concepts such as focus history, z-order, and layout order.
Those are useful, but they are not always the same thing, and they are not always enough to express `next`, `prev`, `last`, and `swap` as one consistent model.

`hyprstack` provides a workspace-local stack model for that.

## What It Does

`hyprstack` is intended for use cases such as:

- treating `next`, `prev`, and `last` as stable operations
- accessing window order from keybinds
- working with a stack model that does not depend on layout or taskbar order

To support that, it provides:

- a stable order per workspace
- access to the current and previous window
- query APIs for stack inspection
- dispatcher APIs for stack actions

## Core Model

Each workspace has its own stack.

Each stack contains:

- a current window
- a previous window
- a stable ordered list of windows

This makes it possible to treat window order and window actions through the same model.

## Query API

The query side is intended for scripts, debugging, and integrations.

- `hyprctl hyprstack stack list`
- `hyprctl hyprstack stack current`
- `hyprctl hyprstack stack around`

Example output:

```json
{
  "workspace": {
    "id": 1,
    "name": "1"
  },
  "current": "0x1234567890",
  "last": "0x2345678901",
  "windows": [
    { "index": 0, "address": "0x1234567890", "class": "com.mitchellh.ghostty", "title": "tmux" },
    { "index": 1, "address": "0x2345678901", "class": "thunar", "title": "home" }
  ]
}
```

## Dispatcher API

The dispatcher side is intended for direct keybind integration.

- `stackfocus, next`
- `stackfocus, prev`
- `stackfocus, last`
- `stackswap, next`
- `stackswap, prev`

Example config:

```ini
bind = $mainMod, J, stackfocus, next
bind = $mainMod, K, stackfocus, prev
bind = $mainMod, TAB, stackfocus, last

bind = SHIFT $mainMod, J, stackswap, next
bind = SHIFT $mainMod, K, stackswap, prev
```

## Installation

Install it using your preferred Hyprland plugin workflow.

- use `hyprpm`
- build from source and load it manually

## Intended Use Cases

- users who want stable window order per workspace
- users who want stack-based keybinds
- users who do not want to depend on taskbar ordering
