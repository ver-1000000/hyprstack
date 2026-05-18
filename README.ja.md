# hyprstack

<p align="center">
  <img src="./assets/logo.svg" alt="hyprstack logo" width="160" height="160">
</p>

<p align="center">
  <a href="./README.md">English</a> · <a href="./README.ja.md">日本語</a>
</p>

`hyprstack` は、[Hyprland](https://github.com/hyprwm/Hyprland) に workspace ごとの stable な window stack を追加するプラグインです。

Hyprland には focus history、z-order、layout order など複数の順番概念があります。 `hyprstack` はそれらとは別に、keybind から扱いやすい `next`、`prev`、`last`、`swap` のための stack model を提供します。

<p align="center">
  <img src="./assets/hyprstack-demo.gif" alt="hyprstack demo" width="640">
</p>

## 機能

- workspace ごとの stable order を持つ
- current window と last-focused window を扱う
- keybind から `next`、`prev`、`last` に focus できる
- current window と隣接 window を swap できる
- `hyprctl` から stack 情報を query できる

## 導入

`hyprpm` での管理を想定しています。

```sh
hyprpm add https://github.com/ver-1000000/hyprstack.git
hyprpm enable hyprstack
```

Hyprland 起動時に `hyprpm` plugin を読み込んでください。

Lua:

```lua
hl.on("hyprland.start", function()
    hl.exec_cmd("hyprpm reload")
end)
```

`.conf`:

```conf
exec-once = hyprpm reload
```

## Keybind 例

Lua config では `hyprctl hyprstack` 経由で stack 操作を呼べます。

```lua
hl.bind(mainMod .. " + TAB", hl.dsp.exec_cmd("hyprctl hyprstack focus last"))
hl.bind(mainMod .. " + J", hl.dsp.exec_cmd("hyprctl hyprstack focus next"))
hl.bind(mainMod .. " + K", hl.dsp.exec_cmd("hyprctl hyprstack focus prev"))

hl.bind("SHIFT + " .. mainMod .. " + J", hl.dsp.exec_cmd("hyprctl hyprstack swap next"))
hl.bind("SHIFT + " .. mainMod .. " + K", hl.dsp.exec_cmd("hyprctl hyprstack swap prev"))
```

`.conf` では dispatcher を直接呼べます。

```ini
bind = $mainMod, TAB, stackfocus, last
bind = $mainMod, J, stackfocus, next
bind = $mainMod, K, stackfocus, prev

bind = SHIFT $mainMod, J, stackswap, next
bind = SHIFT $mainMod, K, stackswap, prev
```

## Query

script や debug 用に stack 状態を取得できます。

```sh
hyprctl hyprstack stack list
hyprctl hyprstack stack current
hyprctl hyprstack stack around
```

`stack around` は stable order 上の `prev` / `next` と、focus history 由来の `last` を返します。

## Floating Stack

`hyprstack` は window の順番と focus/swap 操作だけを扱います。floating 化や表示サイズは Hyprland config 側で決めてください。

floating stack では `maximize` / `fullscreen` より、通常の floating window として大きめに開く設定が安全です。

```lua
hl.window_rule({
    name = "floating-stack",
    match = { class = ".*" },
    float = true,
    center = true,
    size = { "monitor_w * 0.96", "monitor_h * 0.94" }, -- barや外周用に少し余白を残す
})
```
