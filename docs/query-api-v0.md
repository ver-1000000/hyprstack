# hyprstack Query API v0

## 目的

`hyprstack` は、Hyprland の既存の順番概念(focus history、z-order、layout order、taskbar order 風の見え方)をそのまま流用せず、workspace ごとの stable な window stack を plugin 側で提供する

v0 では、Query API と最小の focus dispatcher を提供する

- stack の source of truth は plugin が持つ
- taskbar 順には依存しない
- `focusHistoryID` は `last` の補助候補ではあるが、stable order そのものの根拠にはしない
- `stableID` は起動中の単純増分に見えるため、stable order の根拠にはしない

## v0 の対象

- `stack list`
- `stack current`
- `stack around`
- `stackfocus, next`
- `stackfocus, prev`
- `stackfocus, last`

## command surface

- `hyprctl plugin hyprstack stack list`
- `hyprctl plugin hyprstack stack current`
- `hyprctl plugin hyprstack stack around`
- `stackfocus, next`
- `stackfocus, prev`
- `stackfocus, last`

## stack list

`stack list` は、その workspace の stack 全体を返す

返却は常に JSON とする

```json
{
  "workspace": {
    "id": 1,
    "name": "1"
  },
  "current": "0x1234567890",
  "last": "0x2345678901",
  "windows": [
    {
      "index": 0,
      "address": "0x1234567890",
      "class": "com.mitchellh.ghostty",
      "title": "tmux"
    },
    {
      "index": 1,
      "address": "0x2345678901",
      "class": "thunar",
      "title": "home"
    }
  ]
}
```

### field 定義

- `workspace`
- `workspace.id`
  - Hyprland workspace id
- `workspace.name`
  - Hyprland workspace name
- `current`
  - 現在 active な window の address
  - window がなければ `null`
- `last`
  - `current` のひとつ前に active だった window の address
  - なければ `null`
- `windows`
  - stable stack order の配列
- `windows[].index`
  - `0` 始まりの stack index
- `windows[].address`
  - window address
- `windows[].class`
  - window class
- `windows[].title`
  - window title

### 空 stack の扱い

active workspace が解決できていて、その workspace に window がない場合は `workspace` を残す

```json
{
  "workspace": {
    "id": 9,
    "name": "9"
  },
  "current": null,
  "last": null,
  "windows": []
}
```

active workspace 自体が解決できない場合は `workspace` は `null` になる

```json
{
  "workspace": null,
  "current": null,
  "last": null,
  "windows": []
}
```

## stack current

`stack current` は、現在の workspace における current と last を返す

```json
{
  "workspace": {
    "id": 1,
    "name": "1"
  },
  "current": "0x1234567890",
  "last": "0x2345678901"
}
```

空の場合:

```json
{
  "workspace": null,
  "current": null,
  "last": null
}
```

## stack around

`stack around` は、current を中心に前後関係を返す

```json
{
  "workspace": {
    "id": 1,
    "name": "1"
  },
  "current": "0x1234567890",
  "prev": "0x3456789012",
  "next": "0x2345678901",
  "last": "0x2345678901"
}
```

### around の意味

- `prev`
  - stable stack order 上で current のひとつ前
- `next`
  - stable stack order 上で current のひとつ後
- `last`
  - stable stack order とは独立した、直前に focus されていた window

`prev` と `last` は同じとは限らない

## stackfocus

`stackfocus` は stable stack model に基づいて focus を移動する

- `stackfocus, next`
  - stable stack order 上で次の window へ移動する
  - 末尾では先頭へ wrap する
- `stackfocus, prev`
  - stable stack order 上で前の window へ移動する
  - 先頭では末尾へ wrap する
- `stackfocus, last`
  - stable stack order とは独立した last-focused window へ移動する
