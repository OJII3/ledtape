# ledtape

XIAO ESP32C3 + WS2812 LED テープの自動再生ファームウェア。Nix devShell で PlatformIO を使う。

## 配線

```
[USB 5V]            [外部 5V 電源]
   |                       |
   +----[XIAO ESP32C3]-----+----[GND 共通]----[WS2812 LED テープ GND]
                                |
                          +-----+
                          |
                     [D0 (= GPIO2)]
                          |
                  [WS2812 LED テープ DIN]
```

- 5V 外部電源を LED テープへ供給する
- XIAO の GND と LED 電源の GND を必ず共通化する
- D0 (GPIO2) → LED テープ DIN
- データピン番号を変えるには `src/main.cpp` の `LED_PIN` を編集する
- LED 数を変更するには `src/main.cpp` の `NUM_LEDS` を編集する
- 輝度を変更するには `src/main.cpp` の `BRIGHTNESS` を編集する

## ビルド

```sh
nix develop
pio run -e esp32c3
```

## 書き込み

```sh
nix develop
pio run -e esp32c3 -t upload
```

## ホスト単体テスト

```sh
nix develop
pio test -e native
```

`pio test -e native` はホストの x86_64/aarch64 Linux 上でパターンの色計算ロジックを検証する。実機なしで CI できる。

## 色反転時の対処

WS2812 は `GRB` 順で送出する実装が多いが、稀に赤/緑が入れ替わって見える場合がある。
その場合は `src/main.cpp` の `FastLED.addLeds<WS2812, LED_PIN, GRB>` を `RGB` に変更する。

## 自動再生されるパターン

入力 UI はなく、電源投入後に以下をローテーションで再生する (各 5 秒):

1. `rainbow` — LED 全体に虹色を割り当てる
2. `breathing` — 単色 (赤) をゆっくり明滅
3. `color wipe` — 単色 (青) を端から順に点灯し、消えていく

## 安全上の注意 (PDF 由来)

- LED 1 個あたりの絶対最大は 5V を超えない
- ロール状に巻いたまま点灯しない
- 全白 × 高輝度の長時間点灯は避ける
- 5m を超える長尺は電源注入を検討
- USB 給電だけで多数 LED を駆動しない

## 開発環境の構成

- `flake.nix` — flake-parts で `devShells.default` に `platformio` を入れる
- `platformio.ini` — `esp32c3` (実機) と `native` (ホスト単体テスト) の 2 環境
- `src/main.cpp` — Arduino スケッチ。FastLED 初期化とパターンのローテーション
- `lib/patterns/` — portable な色計算ロジック。`pio test -e native` から直接テストされる
- `test/test_patterns/` — Unity ベースの単体テスト
