# hyprstack Query API v0

## 目的

`hyprstack` は、Hyprland の既存の順番概念(focus history、z-order、layout order、taskbar order 風の見え方)をそのまま流用せず、workspace ごとの stable な window stack を plugin 側で提供する

v0 では、まず Query API を先に固定する

- dispatcher は後回し
- stack の source of truth は plugin が持つ
- taskbar 順には依存しない
- `focusHistoryID` は `last` の補助候補ではあるが、stable order そのものの根拠にはしない
- `stableID` は起動中の単純増分に見えるため、stable order の根拠にはしない

## v0 の対象

- `stack list`
- `stack current`
- `stack around`

## command surface

README 上の公開 API イメージは次を維持する

- `hyprctl plugin hyprstack stack list`
- `hyprctl plugin hyprstack stack current`
- `hyprctl plugin hyprstack stack around`

ただし、2026-04-10 時点のローカル環境(Hyprland 0.54.3)では、`hyprctl` binary から plugin 独自 command を直接叩く経路は確認できていない

確認できたのは、Hyprland socket に対する `hyprstack ...` request である

```sh
printf 'hyprstack stack list' | nc -U "$XDG_RUNTIME_DIR/hypr/$HYPRLAND_INSTANCE_SIGNATURE/.socket.sock"
```

skeleton 実装では plugin 側で `hyprstack` command handler を登録する

- `hyprctl plugin hyprstack ...` は現時点では未実現
- README にある command surface は将来の公開 API イメージとして扱う

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

## skeleton 実装方針

最初の skeleton では、上の JSON shape を先に固定する

- `stack list` は空 stack JSON を返す
- `stack current` は空 current JSON を返す
- `stack around` は空 around JSON を返す
- 実際の window tracking は次段で追加する
