# hyprstack

<p align="center">
  <img src="./assets/logo.svg" alt="hyprstack logo" width="160" height="160">
</p>

<p align="center">
  <a href="./README.md">English</a> · <a href="./README.ja.md">日本語</a>
</p>

`hyprstack` は、[Hyprland](https://github.com/hyprwm/Hyprland) に workspace ごとの stable な window stack を追加するプラグインです。

Hyprland には focus history、z-order、layout order など複数の順番概念があります。
`hyprstack` はそれらとは別に、keybind から扱いやすい `next`、`prev`、`last`、`swap` のための stack model を提供します。

<p align="center">
  <img src="./assets/hyprstack-demo.gif" alt="hyprstack demo" width="640">
</p>

## 機能

- workspace ごとの stable order を持つ
- current window と last-focused window を扱う
- keybind から `next`、`prev`、`last` に focus できる
- current window と隣接 window を swap できる
- `hyprctl` から stack 情報を query できる

## バージョン

`hyprstack` の version は `<動作確認したHyprland version>.<build>` です。
たとえば `0.56.0.1` は、Hyprland `0.56.0` で動作確認した build `1` を表します。

## 対応バージョン

現行リリースはHyprland 0.57.0向けで、0.56.2でのソース互換性も維持しています。
プラグインは実行するHyprlandと同じ環境でビルドしてください。バイナリは共用できません。
Hyprland 0.57向けのビルドにはC++26対応コンパイラが必要です。

Hyprland 0.57では従来のplugin dispatcher登録が廃止されています。
下記のLua keybind例(`hyprctl hyprstack focus/swap`)を使ってください。
`stackfocus` / `stackswap` dispatcherは0.56.2向けのビルドで引き続き利用できます。

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

Hyprland 0.56のみ(`.conf`):

```conf
exec-once = hyprpm reload
```

## Hyprland更新後

`hyprstack` はHyprland内部のplugin APIに対してビルドします。
Hyprlandを更新した後は、pluginを使う前に起動中のHyprlandセッションから次を実行してください。

```sh
hyprpm update
hyprpm reload
hyprctl plugin list
```

`hyprpm update` でビルドに失敗した場合は、`hyprpm update -v` の出力を確認してください。
未対応のAPI変更だけでなく、依存パッケージやビルド環境の問題でも失敗します。
以前のbinaryを強制ロードしないでください。

依存ライブラリのABI判定には、ビルド時にpkg-configで取得したversionを使います。
hyprpmが再利用するHyprlandの`version.h`には、依存更新前の情報が残ることがあるためです。
Hyprland自体のcommit照合と、ロード時のABI不一致の拒否は維持しています。

## Keybind 例

Lua config では `hyprctl hyprstack` 経由で stack 操作を呼べます。

```lua
hl.bind(mainMod .. " + TAB", hl.dsp.exec_cmd("hyprctl hyprstack focus last"))
hl.bind(mainMod .. " + J", hl.dsp.exec_cmd("hyprctl hyprstack focus next"))
hl.bind(mainMod .. " + K", hl.dsp.exec_cmd("hyprctl hyprstack focus prev"))

hl.bind("SHIFT + " .. mainMod .. " + J", hl.dsp.exec_cmd("hyprctl hyprstack swap next"))
hl.bind("SHIFT + " .. mainMod .. " + K", hl.dsp.exec_cmd("hyprctl hyprstack swap prev"))
```

Hyprland 0.56の`.conf`ではdispatcherを直接呼べます。

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

Hyprland 0.57では`workspace.id`が、番号付きworkspaceなら数値、名前付き・special workspaceなら
識別用の文字列(例: `"notes"`、`"special:notes"`)になります。`workspace.name`は表示名です。
0.56では従来どおり数値を返します。スクリプトでは数値と文字列の両方を受け入れてください。

## Floating Stack

`hyprstack` は window の順番と focus/swap 操作だけを扱います。 floating 化や表示サイズは Hyprland config 側で決めてください。

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
