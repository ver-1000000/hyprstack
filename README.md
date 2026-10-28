# hyprstack

<p align="center">
  <img src="./assets/logo.svg" alt="hyprstack logo" width="160" height="160">
</p>

<p align="center">
  <a href="./README.md">English</a> · <a href="./README.ja.md">日本語</a>
</p>

`hyprstack` is a [Hyprland](https://github.com/hyprwm/Hyprland) plugin that adds a stable per-workspace window stack.

Hyprland already has focus history, z-order, layout order, and other ordering concepts. `hyprstack` provides a separate stack model for keybind-friendly `next`, `prev`, `last`, and `swap` operations.

<p align="center">
  <img src="./assets/hyprstack-demo.gif" alt="hyprstack demo" width="640">
</p>

## Features

- Keeps a stable order per workspace
- Tracks the current window and the last-focused window
- Focuses `next`, `prev`, and `last` from keybinds
- Swaps the current window with its neighbor
- Exposes stack state through `hyprctl`

## Versioning

`hyprstack` versions use `<tested Hyprland version>.<build>`.
For example, `0.56.0.1` means build `1`, tested with Hyprland `0.56.0`.

## Compatibility

The current release targets Hyprland 0.57.0 and retains source compatibility with 0.56.2.
Build the plugin against the exact Hyprland installation you run; binaries are not interchangeable.
Hyprland 0.57 builds require a C++26-capable compiler.

Hyprland 0.57 no longer registers legacy plugin dispatchers. Use the Lua keybind examples
below (`hyprctl hyprstack focus/swap`); direct `stackfocus` / `stackswap` dispatchers
remain available when built for 0.56.2.

## Installation

`hyprstack` is intended to be managed with `hyprpm`.

```sh
hyprpm add https://github.com/ver-1000000/hyprstack.git
hyprpm enable hyprstack
```

Load `hyprpm` plugins when Hyprland starts.

Lua:

```lua
hl.on("hyprland.start", function()
    hl.exec_cmd("hyprpm reload")
end)
```

Hyprland 0.56 only (`.conf`):

```conf
exec-once = hyprpm reload
```

## After Updating Hyprland

`hyprstack` is built against Hyprland's internal plugin API.
After every Hyprland update, run the following from the running Hyprland session before relying on the plugin.

```sh
hyprpm update
hyprpm reload
hyprctl plugin list
```

If a build fails, inspect `hyprpm update -v`: unsupported API changes, missing dependencies,
and build environment problems can all cause failures.
Do not force-load the previous binary.

Dependency ABI versions come from pkg-config at build time, since Hyprland's cached
`version.h` can describe dependencies from an older build. The Hyprland commit check
and rejection of mismatched ABIs remain enabled.

## Keybind Examples

Lua configs can call stack actions through `hyprctl hyprstack`.

```lua
hl.bind(mainMod .. " + TAB", hl.dsp.exec_cmd("hyprctl hyprstack focus last"))
hl.bind(mainMod .. " + J", hl.dsp.exec_cmd("hyprctl hyprstack focus next"))
hl.bind(mainMod .. " + K", hl.dsp.exec_cmd("hyprctl hyprstack focus prev"))

hl.bind("SHIFT + " .. mainMod .. " + J", hl.dsp.exec_cmd("hyprctl hyprstack swap next"))
hl.bind("SHIFT + " .. mainMod .. " + K", hl.dsp.exec_cmd("hyprctl hyprstack swap prev"))
```

On Hyprland 0.56, `.conf` configs can call dispatchers directly.

```ini
bind = $mainMod, TAB, stackfocus, last
bind = $mainMod, J, stackfocus, next
bind = $mainMod, K, stackfocus, prev

bind = SHIFT $mainMod, J, stackswap, next
bind = SHIFT $mainMod, K, stackswap, prev
```

## Query

Use these commands for scripts and debugging.

```sh
hyprctl hyprstack stack list
hyprctl hyprstack stack current
hyprctl hyprstack stack around
```

`stack around` returns stable-order `prev` / `next` and focus-history-based `last`.

On Hyprland 0.57, `workspace.id` is a number for numbered workspaces and an
addressable string for named/special workspaces (for example, `"notes"` or
`"special:notes"`). `workspace.name` is the display name. On 0.56, IDs remain numeric.
Scripts should accept both JSON number and string IDs.

## Floating Stack

`hyprstack` only handles window order and focus/swap actions. Whether windows float and how large they are is Hyprland config responsibility.

For floating stack workflows, opening normal floating windows at a large size is safer than using `maximize` / `fullscreen`.

```lua
hl.window_rule({
    name = "floating-stack",
    match = { class = ".*" },
    float = true,
    center = true,
    size = { "monitor_w * 0.96", "monitor_h * 0.94" }, -- leave some room for bars and outer margins
})
```
