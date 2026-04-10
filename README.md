# hyprstack

<p align="center">
  <img src="./assets/logo.svg" alt="hyprstack logo" width="160" height="160">
</p>

<p align="center">
  <a href="./README.md">English</a> · <a href="./README.ja.md">日本語</a>
</p>

`hyprstack` is a plugin for [Hyprland](https://github.com/hyprwm/Hyprland) that adds a stable per-workspace window stack.

The goal of `hyprstack` is to make window order a first-class concept.

Hyprland already has multiple ordering concepts such as focus history, z-order, and layout order.
Those are useful, but they are not always the same thing, and they are not always enough to express `next`, `prev`, `last`, and `swap` as one consistent model.

`hyprstack` provides a workspace-local stack model for that.

<p align="center">
  <img src="./assets/hyprstack-demo.gif" alt="hyprstack demo" width="640">
</p>

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

`stack around` returns stable-order neighbors as `prev` and `next`, while `last` remains the last-focused window independently of stable order.

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

`stackswap` swaps the current window with its stable-order neighbor, wraps at the ends, and updates the resulting order for subsequent stack actions.

Example config:

```ini
bind = $mainMod, J, stackfocus, next
bind = $mainMod, K, stackfocus, prev
bind = $mainMod, TAB, stackfocus, last

bind = SHIFT $mainMod, J, stackswap, next
bind = SHIFT $mainMod, K, stackswap, prev
```

## Installation

### Prerequisites

On Arch Linux, install the packages needed for building and loading the plugin:

```sh
$ sudo pacman -S --needed git base-devel cmake cpio pkgconf hyprland
```

### Install with `hyprpm`

```sh
$ hyprpm add https://github.com/ver-1000000/hyprstack.git
$ hyprpm enable hyprstack  # enable hyprstack
```

If your Hyprland config does not already load `hyprpm`-managed plugins at startup, add the following line:

```conf
exec-once = hyprpm reload
```

### Install manually (build from source)

```sh
$ git clone https://github.com/ver-1000000/hyprstack.git
$ cd hyprstack
$ make all                                       # build the plugin
$ hyprctl plugin load "$PWD/build/hyprstack.so"  # load the built plugin
```

Then add the following line to your Hyprland config and reload:

```conf
plugin = /absolute/path/to/build/hyprstack.so
```
