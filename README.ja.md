# hyprstack

`hyprstack` は、Hyprland に workspace ごとの stable な window stack を追加するプラグインです。

`hyprstack` の目的は、window の順番を一貫した概念として扱えるようにすることです。

Hyprland にはすでに、focus history、z-order、layout order など複数の順番概念があります。
しかし、それらは必ずしも同じものではなく、`next`、`prev`、`last`、`swap` のような操作を安定したひとつのモデルとして扱うには足りないことがあります。

`hyprstack` は、そのための workspace-local な stack model を提供します。

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

- `hyprctl plugin hyprstack stack list`
- `hyprctl plugin hyprstack stack current`
- `hyprctl plugin hyprstack stack around`

出力イメージ:

```json
{
  "workspace": 1,
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

設定例:

```ini
bind = $mainMod, J, stackfocus, next
bind = $mainMod, K, stackfocus, prev
bind = $mainMod, TAB, stackfocus, last

bind = SHIFT $mainMod, J, stackswap, next
bind = SHIFT $mainMod, K, stackswap, prev
```

## 導入

Hyprland plugin の一般的な方法で導入してください。

- `hyprpm` を使う
- source から build して手動で読み込む
