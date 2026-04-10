# hyprstack Manual Test Cases

## 前提

- Hyprland 上で `hyprstack-dev.so` を load 済み
- `scripts/query.sh` が実行できる

注意:

- 2026-04-10 時点のこの環境では、`hyprctl plugin unload/load` による hot reload で Hyprland が crash した
- `hyprpm` は state store 初期化に superuser 権限が必要で、まだ開発フローに組み込めていない

## 基本確認

1. plugin が load 済みであることを確認する
2. `scripts/query.sh stack list` を実行する
3. `scripts/query.sh stack current` を実行する
4. `scripts/query.sh stack around` を実行する

期待:

- JSON が返る
- `workspace.id` と `workspace.name` が active workspace と一致する
- `current` が active window の address と一致する

## 2 window focus

1. 同じ workspace に window を 2 つ開く
2. A を focus する
3. B を focus する
4. `scripts/query.sh stack current` を実行する
5. `scripts/query.sh stack around` を実行する

期待:

- `current` は B
- `last` は A
- `prev` と `next` は stable order に従う

## 3 window focus roundtrip

1. 同じ workspace に A, B, C を並べる
2. A -> B -> C -> B の順で focus する
3. `scripts/query.sh stack list` を実行する
4. `scripts/query.sh stack around` を実行する

期待:

- `windows` は stable order を保つ
- `current` は B
- `last` は C
- `prev` と `next` は stable order 上の隣接を返す

## close

1. 同じ workspace に A, B を置く
2. A -> B の順で focus する
3. B を閉じる
4. `scripts/query.sh stack list` を実行する

期待:

- `windows` から B が消える
- `current` は active window と一致する
- 閉じた window が `last` に残り続けない

## workspace move

1. workspace 1 に A, B を置く
2. B を workspace 2 へ移す
3. workspace 1 で `scripts/query.sh stack list` を実行する
4. workspace 2 に移動して `scripts/query.sh stack list` を実行する

期待:

- workspace 1 では B が消える
- workspace 2 では B が見える
- 各 workspace の `windows` は独立している
