# LED Tape Firmware on Xiao ESP32C3

- 日付: 2026-07-07
- ステータス: 設計ドラフト
- 関連資料: 添付 PDF `S8f52cdbede8a420294b5ad9010043d0eZ.pdf`

## 目的

Seeed XIAO ESP32C3 から WS2812 LED テープを制御するファームウェアの、最初の動作確認までを成立させる。
入力 UI は持たず、電源投入後に自動でパターンを再生する。
開発環境は Nix の devShell で再現可能とし、ビルド・書き込みの入口を README にまとめる。

## スコープ

含む:

- Nix 開発環境 (`flake.nix` + `flake-parts`)
- PlatformIO プロジェクト設定 (`platformio.ini`)
- Arduino + FastLED による自動再生ファームウェア (`src/main.cpp`)
- 配線・運用上の注意をまとめた `README.md`
- 検証コマンド

含まない:

- Wi-Fi 経由の制御
- MQTT / Web UI / REST
- ボタンやシリアル入力による操作
- パターンエディタや永続設定

## ハードウェア前提

- マイコン: Seeed XIAO ESP32C3
- LED テープ: WS2812 (データシート上は WS2812B / WS2812C 系を想定)
- 電源: 5V 外部電源を LED テープへ供給し、XIAO の GND と LED 電源の GND は必ず共通化する
- 信号線: XIAO の GPIO (D0 相当を初期値) → LED テープ DIN
  - データピン番号は `src/main.cpp` の定数 `LED_PIN` だけで変更できる
- LED 長さ: 初期値 `30`、定数 `NUM_LEDS` で変更

### 安全上の注意 (PDF 由来)

- LED 1 個の絶対最大は 5V を超えない
- ロール状に巻いたまま点灯しない
- 全白かつ高輝度の長時間点灯を避ける (発熱と劣化)
- 5m を超える長尺は電源注入を検討
- USB 給電だけで多数 LED を駆動しない

## ソフトウェア構成

### スタック

- 言語: C++ (Arduino API)
- ライブラリ: `FastLED`
- フレームワーク: Arduino
- ビルド/書き込み: PlatformIO Core
- 開発環境: `nix develop` (flake-parts の `perSystem.devShells.default`)

### ファイル構成

```
flake.nix
platformio.ini
src/main.cpp
README.md
docs/superpowers/specs/2026-07-07-ledtape-firmware-design.md
```

## 設計

### 1. `flake.nix`

- `nixpkgs` と `flake-parts` を input とする
- `flake-parts.lib.mkFlake` を使う
- `systems` は `x86_64-linux` と `aarch64-linux`
- `perSystem.devShells.default.packages` に `platformio` を含める
- `perSystem.formatter` は `pkgs.nixfmt-rfc-style` を想定

`perSystem` を使う理由は、ボード・ツールチェインに依存する内容をシステム単位で持たせ、将来 macOS やその他のホストにも同じ `devShell` の枠組みで展開できるようにするため。

### 2. `platformio.ini`

- `platform = espressif32`
- `framework = arduino`
- `board` は XIAO ESP32C3 用の PlatformIO board id を設定
- `lib_deps = fastled/FastLED`
- `monitor_speed = 115200`
- 書き込みポートは固定せず、PlatformIO の自動検出に任せる

### 3. ファームウェア `src/main.cpp`

公開する定数:

- `LED_PIN` — データピン (初期値: `D0` 相当の GPIO 番号)
- `NUM_LEDS` — LED 数 (初期値: `30`)
- `BRIGHTNESS` — 輝度 (初期値: `32` / 255)
- `LED_COLOR_ORDER` — 初期値 `GRB` (WS2812 で一般的。色が反転したら `RGB` に変更)

`setup()`:

- `FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS)` で初期化
- `FastLED.setBrightness(BRIGHTNESS)`
- 起動確認として短い赤・緑・青点灯を順次実施 (各 200ms 程度)

`loop()`:

- 入力がないので以下のパターンを時間ローテーションで自動再生
  - `rainbow`: 虹色スクロール
  - `breathing`: 単色をゆっくりフェード
  - `color wipe`: 単色を端から順に点灯
- 全白・高輝度パターンは含めない
- 各パターンは関数として分割し、`loop()` から一定時間ごとに呼び出す
- `delay()` ベースの単純実装に留め、初回動作確認を優先

拡張性:

- パターンを追加するときは `pattern_*` 関数を 1 つ足し、ローテーション配列に 1 行追加するだけで済む構造

### 4. `README.md`

下記を最低限まとめる:

- 配線図 (テキスト)
- `nix develop` での入り方
- `pio run` / `pio run -t upload`
- `LED_PIN`, `NUM_LEDS`, `BRIGHTNESS`, `LED_COLOR_ORDER` の変更箇所
- 色が反転した場合の `RGB` / `GRB` 切替手順
- PDF 由来の安全注意

## 検証

- `nix flake check` — flake の整合性
- `nix develop -c pio run` — ビルドが通ること
- 実機接続時に `nix develop -c pio run -t upload` で書き込み
- LED 側の確認は目視
  - 起動直後の赤・緑・青が見える
  - パターンが途中で固まらない
  - 長時間点灯で 1 個だけ極端に明るい (全白) 状態が発生しない

## リスク

- PlatformIO の XIAO ESP32C3 ボード id や USB 検出: 環境差がある。失敗時は README に復旧手順を書く。
- FastLED のバージョン差: `FastLED.h` の API 変化に依存しすぎないよう、初回は最小 API のみ使う。
- Nix 環境での `platformio` のパッケージは nixpkgs の更新に依存する。動かない場合は README にパッケージ切り替え手順を残す。

## セルフレビュー

- プレースホルダ: なし
- 内部矛盾: なし
- スコープ: 単一の自己完結した実装計画に収まる
- 曖昧さ:
  - `LED_PIN` の初期値 (GPIO 番号) は、PlatformIO / ESP32 Arduino コアのピン番号で確定する。実装計画フェーズで XIAO ESP32C3 の対応 GPIO を確認する。
  - パターン関数の名前は実装計画フェーズで決定する。
