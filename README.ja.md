# hyprstack

<p align="center">
  <img src="./assets/logo.svg" alt="hyprstack logo" width="160" height="160">
</p>

<p align="center">
  <a href="./README.md">English</a> · <a href="./README.ja.md">日本語</a>
</p>

`hyprstack` は、[Hyprland](https://github.com/hyprwm/Hyprland) に workspace ごとの stable な window stack を追加するプラグインです。

`hyprstack` の目的は、window の順番を一貫した概念として扱えるようにすることです。

Hyprland にはすでに、focus history、z-order、layout order など複数の順番概念があります。
しかし、それらは必ずしも同じものではなく、`next`、`prev`、`last`、`swap` のような操作を安定したひとつのモデルとして扱うには足りないことがあります。

`hyprstack` は、そのための workspace-local な stack model を提供します。

<p align="center">
  <img src="./assets/hyprstack-demo.gif" alt="hyprstack demo" width="640">
</p>

## できること

`hyprstack` は、たとえば次のような用途を想定しています。

- `next`、`prev`、`last` を一貫した意味で扱いたい
- keybind から stable な window order にアクセスしたい
- layout や taskbar の都合と切り離して window stack を扱いたい

そのために、次の機能を提供します。

- workspace ごとの stable order を持つ
- 現在の window と直前の window を扱う
- Query API で stack 情報を取得できる
- Dispatcher API で stack 操作を呼べる

## 基本モデル

各 workspace は独自の stack を持ちます。

各 stack は次を持ちます。

- 現在の window
- ひとつ前の window
- 安定した順序を持つ window 一覧

これにより、window の並びと操作を同じモデルで扱えるようにします。

## Query API

script や debug 用の読み取り API です。

- `hyprctl hyprstack stack list`
- `hyprctl hyprstack stack current`
- `hyprctl hyprstack stack around`

`stack around` の `prev` と `next` は stable order 上の隣接を返し、`last` は stable order とは独立した直前の focused window を返します。

出力イメージ:

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

keybind から直接使うための操作 API です。

- `stackfocus, next`
- `stackfocus, prev`
- `stackfocus, last`
- `stackswap, next`
- `stackswap, prev`

`stackswap` は current window を stable order 上の隣接 window と入れ替え、端では wrap し、その結果を以後の stack 操作にも反映します。

設定例:

```ini
bind = $mainMod, J, stackfocus, next
bind = $mainMod, K, stackfocus, prev
bind = $mainMod, TAB, stackfocus, last

bind = SHIFT $mainMod, J, stackswap, next
bind = SHIFT $mainMod, K, stackswap, prev
```

## 導入

### `hyprpm` で導入する

`hyprstack` は `hyprpm` での管理を想定したプラグインです。

次の手順でインストールと有効化を行ってください。

```sh
$ hyprpm add https://github.com/ver-1000000/hyprstack.git
$ hyprpm enable hyprstack  # hyprstack を有効化
```

`hyprpm` 管理の plugin を起動時に読み込む設定がまだ無い場合は、Hyprland config に次の一行を追加してください。

```conf
exec-once = hyprpm reload
```

### 手動で導入する (開発用)

local build をそのまま読み込むための手順です。

```sh
$ git clone https://github.com/ver-1000000/hyprstack.git
$ cd hyprstack
$ make all                                       # plugin を build
$ hyprctl plugin load "$PWD/build/hyprstack.so"  # build した plugin を読み込む
```

次に、Hyprland config に次の一行を追加して reload してください。

```conf
plugin = /absolute/path/to/build/hyprstack.so
```
