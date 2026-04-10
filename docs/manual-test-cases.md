# hyprstack Manual Test Cases

## 前提

- Hyprland 上で `hyprstack.so` を load 済み
- Query API と dispatcher を実行できる

## 基本確認

1. plugin が load 済みであることを確認する
2. `hyprctl plugin hyprstack stack list` を実行する
3. `hyprctl plugin hyprstack stack current` を実行する
4. `hyprctl plugin hyprstack stack around` を実行する

期待:

- JSON が返る
- `workspace.id` と `workspace.name` が active workspace と一致する
- `current` が active window の address と一致する

## empty workspace

1. active workspace を空の workspace に切り替える
2. `hyprctl plugin hyprstack stack list` を実行する
3. `hyprctl plugin hyprstack stack current` を実行する

期待:

- `workspace.id` と `workspace.name` は active workspace を返す
- `windows` は空配列になる
- `current` と `last` は `null` になる

## 2 window focus

1. 同じ workspace に window を 2 つ開く
2. A を focus する
3. B を focus する
4. `hyprctl plugin hyprstack stack current` を実行する
5. `hyprctl plugin hyprstack stack around` を実行する

期待:

- `current` は B
- `last` は A
- `prev` と `next` は stable order に従う

## 3 window focus roundtrip

1. 同じ workspace に A, B, C を並べる
2. A -> B -> C -> B の順で focus する
3. `hyprctl plugin hyprstack stack list` を実行する
4. `hyprctl plugin hyprstack stack around` を実行する

期待:

- `windows` は stable order を保つ
- `current` は B
- `last` は C
- `prev` と `next` は stable order 上の隣接を返す

## stackfocus

1. 同じ workspace に A, B を置く
2. A -> B の順で focus する
3. `hyprctl dispatch stackfocus last` を実行する
4. `hyprctl dispatch stackfocus next` を実行する
5. `hyprctl dispatch stackfocus prev` を実行する

期待:

- `stackfocus last` で A に戻る
- `stackfocus next` と `stackfocus prev` は stable order 上を巡回する
- 先頭と末尾では wrap する

## close

1. 同じ workspace に A, B を置く
2. A -> B の順で focus する
3. B を閉じる
4. `hyprctl plugin hyprstack stack list` を実行する

期待:

- `windows` から B が消える
- `current` は active window と一致する
- 閉じた window が `last` に残り続けない

## workspace move

1. workspace 1 に A, B を置く
2. B を workspace 2 へ移す
3. workspace 1 で `hyprctl plugin hyprstack stack list` を実行する
4. workspace 2 に移動して `hyprctl plugin hyprstack stack list` を実行する

期待:

- workspace 1 では B が消える
- workspace 2 では B が見える
- 各 workspace の `windows` は独立している
