# LED Tape Firmware Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 入力なしの自動再生ファームウェアを Xiao ESP32C3 + WS2812 + Arduino + FastLED + PlatformIO でビルド・書き込みできる状態にする。Nix devShell で再現可能な開発環境も整備する。

**Architecture:**
- `flake-parts` ベースの `flake.nix` で `perSystem.devShells.default` に `platformio` を入れる
- `platformio.ini` に `esp32c3` (実機) と `native` (ホスト単体テスト) の 2 環境
- 色計算ロジックは `src/patterns.*` の純粋関数として切り出し、`native` 環境で TDD
- LED ハードウェア依存部 (`src/main.cpp`) は `patterns` を呼ぶだけにする
- `loop()` は `delay()` ベースでパターンをローテーション再生
- 全白や高輝度を避ける安全側の既定値

**Tech Stack:** Nix (flake + flake-parts), PlatformIO, Arduino framework (espressif32), FastLED, Unity (PlatformIO 同梱)

**Spec:** `docs/superpowers/specs/2026-07-07-ledtape-firmware-design.md`

---

## ファイル構成

| ファイル | 役割 |
| --- | --- |
| `flake.nix` | flake-parts で `devShells.default` (platformio) を提供 |
| `.gitignore` | PlatformIO / Nix 生成物を除外 |
| `platformio.ini` | `esp32c3` (実機) と `native` (ホスト単体テスト) を定義 |
| `src/Color.h` | 自前の `struct Color { uint8_t r, g, b; }` 定義 (FastLED 非依存) |
| `src/patterns.h` | パターン関数の宣言 |
| `src/patterns.cpp` | パターン関数の実装 (色計算は Pure C++) |
| `src/main.cpp` | FastLED 初期化と `setup()` / `loop()` 配線 |
| `test/test_patterns/test_main.cpp` | Unity による単体テスト |
| `README.md` | 配線、ビルド、書き込み、安全注意 |

---

## Task 1: Nix 開発環境を作る

**Files:**
- Create: `flake.nix`
- Create: `.gitignore`

- [ ] **Step 1: `.gitignore` を作成する**

`/.gitignore` を作成して PlatformIO と Nix の生成物を除外する:

```gitignore
.pio/
.pioenvs/
.piodebug/
.piolibdeps/
result*
.direnv/
```

- [ ] **Step 2: `flake.nix` を作成する**

`/flake.nix`:

```nix
{
  description = "LED tape firmware on Xiao ESP32C3";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-parts.url = "github:hercules-ci/flake-parts";
  };

  outputs =
    inputs@{ flake-parts, ... }:
    flake-parts.lib.mkFlake { inherit inputs; } {
      systems = [
        "x86_64-linux"
        "aarch64-linux"
      ];

      perSystem =
        { pkgs, ... }:
        {
          devShells.default = pkgs.mkShell {
            packages = with pkgs; [
              platformio
            ];
            shellHook = ''
              echo "ledtape devShell ready. Try: pio run -e esp32c3"
            '';
          };
        };
    };
}
```

- [ ] **Step 3: flake の整合性を確認する**

Run:
```bash
nix flake check
```

Expected: エラーなしで完了する。`platformio` が解決でき、`nix develop` に必要な属性が揃っている。

- [ ] **Step 4: devShell に入れることを確認する**

Run:
```bash
nix develop -c pio --version
```

Expected: `PlatformIO Core, version ...` のようなバージョン文字列が 1 行で出力される。

- [ ] **Step 5: 変更をコミットする**

```bash
git add .gitignore flake.nix
git commit -m "chore: add Nix devShell with PlatformIO"
```

---

## Task 2: PlatformIO 設定と空スケルトン

**Files:**
- Create: `platformio.ini`
- Create: `src/main.cpp` (中身は空スケルトン)

- [ ] **Step 1: `platformio.ini` を作成する**

`/platformio.ini`:

```ini
[env:esp32c3]
platform = espressif32
board = esp32-c3-devkitm-1
framework = arduino
lib_deps =
    fastled/FastLED
monitor_speed = 115200
build_flags =
    -DARDUINO_XIAO_ESP32C3

[env:native]
platform = native
test_framework = unity
```

注: `esp32-c3-devkitm-1` は PlatformIO の espressif32 プラットフォームで ESP32-C3 系を扱う際に最も互換性が高いボード定義。XIAO ESP32C3 固有の派生ボードは公式に存在しないため `-DARDUINO_XIAO_ESP32C3` を `build_flags` で与えてボード ID 自体は互換品でビルドする。

- [ ] **Step 2: 空の `src/main.cpp` を作成する**

`/src/main.cpp`:

```cpp
#include <Arduino.h>

void setup() {}

void loop() {}
```

- [ ] **Step 3: 実機向けビルドが通ることを確認する**

Run:
```bash
nix develop -c pio run -e esp32c3
```

Expected: `SUCCESS` で終了し、`.pio/build/esp32c3/firmware.elf` が生成される。初回は espressif32 プラットフォームと Arduino コアのダウンロードが入る。

- [ ] **Step 4: 変更をコミットする**

```bash
git add platformio.ini src/main.cpp
git commit -m "chore: add PlatformIO config and empty sketch"
```

---

## Task 3: 色型とテスト基盤 (Red→Green 統合)

**Files:**
- Create: `src/Color.h`
- Create: `test/test_patterns/test_main.cpp`

注: 計画当初は Red→Green の 2 ステップに分けていたが、`Color` / `colorEquals` がヘッダオンリー (struct + inline) のためリンクエラーで Red 状態を構造的に作れない。Task 3 内で Red テストの追加と Green 確認までを 1 コミットに統合する。

- [ ] **Step 1: `src/Color.h` を作成する**

`/src/Color.h`:

```cpp
#pragma once

#include <stddef.h>
#include <stdint.h>

struct Color {
  uint8_t r;
  uint8_t g;
  uint8_t b;
};

inline bool colorEquals(const Color &a, const Color &b) {
  return a.r == b.r && a.g == b.g && a.b == b.b;
}
```

- [ ] **Step 2: テストを追加する**

`/test/test_patterns/test_main.cpp`:

```cpp
#include <unity.h>
#include "Color.h"

void setUp() {}
void tearDown() {}

void test_color_equals_works() {
  Color a{10, 20, 30};
  Color b{10, 20, 30};
  Color c{10, 20, 31};
  TEST_ASSERT_TRUE(colorEquals(a, b));
  TEST_ASSERT_FALSE(colorEquals(a, c));
}

int main(int argc, char **argv) {
  UNITY_BEGIN();
  RUN_TEST(test_color_equals_works);
  return UNITY_END();
}
```

- [ ] **Step 3: テストを実行して Green を確認する**

Run:
```bash
nix develop -c pio test -e native
```

Expected: `test_color_equals_works` が PASS する。失敗する場合は追加・編集ミスを修正してリトライ。

- [ ] **Step 4: 変更をコミットする**

```bash
git add src/Color.h test/test_patterns/test_main.cpp
git commit -m "test: add Color type and equality helper"
```

---

## Task 5: hsv → rgb 変換 (TDD)

**Files:**
- Create: `src/patterns.h`
- Create: `src/patterns.cpp`
- Modify: `test/test_patterns/test_main.cpp`

- [ ] **Step 1: 失敗するテストを追加する**

`/test/test_patterns/test_main.cpp` の末尾、`main` の `RUN_TEST` 群に以下を追加する:

```cpp
void test_hsv_to_rgb_red() {
  Color c = hsvToRgb(0, 255, 255);
  TEST_ASSERT_EQUAL_UINT8(255, c.r);
  TEST_ASSERT_EQUAL_UINT8(0, c.g);
  TEST_ASSERT_EQUAL_UINT8(0, c.b);
}

void test_hsv_to_rgb_green() {
  Color c = hsvToRgb(85, 255, 255);
  TEST_ASSERT_EQUAL_UINT8(0, c.r);
  TEST_ASSERT_EQUAL_UINT8(255, c.g);
  TEST_ASSERT_EQUAL_UINT8(0, c.b);
}

void test_hsv_to_rgb_blue() {
  Color c = hsvToRgb(170, 255, 255);
  TEST_ASSERT_EQUAL_UINT8(0, c.r);
  TEST_ASSERT_EQUAL_UINT8(0, c.g);
  TEST_ASSERT_EQUAL_UINT8(255, c.b);
}
```

`main` 関数の中に以下を追加する:

```cpp
RUN_TEST(test_hsv_to_rgb_red);
RUN_TEST(test_hsv_to_rgb_green);
RUN_TEST(test_hsv_to_rgb_blue);
```

- [ ] **Step 2: 失敗を確認する**

Run:
```bash
nix develop -c pio test -e native
```

Expected: `hsvToRgb` 未定義のリンクエラーで失敗する。

- [ ] **Step 3: 宣言を `src/patterns.h` に追加する**

`/src/patterns.h`:

```cpp
#pragma once

#include "Color.h"

Color hsvToRgb(uint8_t hue, uint8_t sat, uint8_t val);
```

- [ ] **Step 4: 実装を `src/patterns.cpp` に追加する**

`/src/patterns.cpp`:

```cpp
#include "patterns.h"

Color hsvToRgb(uint8_t hue, uint8_t sat, uint8_t val) {
  uint8_t region = hue / 43;
  uint8_t remainder = (hue - (region * 43)) * 6;

  uint8_t p = (val * (255 - sat)) / 255;
  uint8_t q = (val * (255 - ((sat * remainder) / 255))) / 255;
  uint8_t t = (val * (255 - ((sat * (255 - remainder)) / 255))) / 255;

  switch (region) {
    case 0: return Color{val, t, p};
    case 1: return Color{q, val, p};
    case 2: return Color{p, val, t};
    case 3: return Color{p, q, val};
    case 4: return Color{t, p, val};
    default: return Color{val, p, q};
  }
}
```

- [ ] **Step 5: テストが通ることを確認する**

Run:
```bash
nix develop -c pio test -e native
```

Expected: HSV 変換の 3 テストがすべて PASS する。

- [ ] **Step 6: コミットする**

```bash
git add src/patterns.h src/patterns.cpp test/test_patterns/test_main.cpp
git commit -m "feat(patterns): add hsvToRgb conversion"
```

---

## Task 6: rainbow パターン (TDD)

**Files:**
- Modify: `src/patterns.h`
- Modify: `src/patterns.cpp`
- Modify: `test/test_patterns/test_main.cpp`

- [ ] **Step 1: 失敗するテストを追加する**

`/test/test_patterns/test_main.cpp` に追加:

```cpp
void test_rainbow_distributes_hue_across_leds() {
  const size_t n = 4;
  Color leds[4] = {};
  fillRainbow(leds, n, /*startHue=*/0, /*brightness=*/255);
  // 4 灯で hue 0/64/128/192 を割り当てる想定
  Color h0 = hsvToRgb(0, 255, 255);
  Color h64 = hsvToRgb(64, 255, 255);
  Color h128 = hsvToRgb(128, 255, 255);
  Color h192 = hsvToRgb(192, 255, 255);
  TEST_ASSERT_TRUE(colorEquals(leds[0], h0));
  TEST_ASSERT_TRUE(colorEquals(leds[1], h64));
  TEST_ASSERT_TRUE(colorEquals(leds[2], h128));
  TEST_ASSERT_TRUE(colorEquals(leds[3], h192));
}
```

`main` 内の `RUN_TEST` に追加:

```cpp
RUN_TEST(test_rainbow_distributes_hue_across_leds);
```

- [ ] **Step 2: 失敗を確認する**

Run:
```bash
nix develop -c pio test -e native
```

Expected: `fillRainbow` 未定義でリンク失敗する。

- [ ] **Step 3: `src/patterns.h` に宣言を追加する**

`/src/patterns.h` の末尾に追加:

```cpp
void fillRainbow(Color *leds, size_t n, uint8_t startHue, uint8_t brightness);
```

- [ ] **Step 4: `src/patterns.cpp` に実装を追加する**

`/src/patterns.cpp` の末尾に追加:

```cpp
void fillRainbow(Color *leds, size_t n, uint8_t startHue, uint8_t brightness) {
  if (n == 0) return;
  for (size_t i = 0; i < n; ++i) {
    uint8_t hue = startHue + (uint8_t)((i * 256) / n);
    leds[i] = hsvToRgb(hue, 255, brightness);
  }
}
```

- [ ] **Step 5: テストが通ることを確認する**

Run:
```bash
nix develop -c pio test -e native
```

Expected: rainbow テストが PASS する。

- [ ] **Step 6: コミットする**

```bash
git add src/patterns.h src/patterns.cpp test/test_patterns/test_main.cpp
git commit -m "feat(patterns): add rainbow fill"
```

---

## Task 7: breathing パターン (TDD)

**Files:**
- Modify: `src/patterns.h`
- Modify: `src/patterns.cpp`
- Modify: `test/test_patterns/test_main.cpp`

- [ ] **Step 1: 失敗するテストを追加する**

`/test/test_patterns/test_main.cpp` に追加:

```cpp
void test_breathing_uses_supplied_color_at_max_phase() {
  const size_t n = 3;
  Color leds[3] = {};
  Color base{200, 0, 0};
  fillBreathing(leds, n, base, /*phase=*/255);
  TEST_ASSERT_TRUE(colorEquals(leds[0], base));
  TEST_ASSERT_TRUE(colorEquals(leds[1], base));
  TEST_ASSERT_TRUE(colorEquals(leds[2], base));
}

void test_breathing_darkens_at_zero_phase() {
  const size_t n = 2;
  Color leds[2] = {};
  Color base{200, 0, 0};
  fillBreathing(leds, n, base, /*phase=*/0);
  TEST_ASSERT_EQUAL_UINT8(0, leds[0].r);
  TEST_ASSERT_EQUAL_UINT8(0, leds[0].g);
  TEST_ASSERT_EQUAL_UINT8(0, leds[0].b);
  TEST_ASSERT_EQUAL_UINT8(0, leds[1].r);
}
```

`main` 内の `RUN_TEST` に追加:

```cpp
RUN_TEST(test_breathing_uses_supplied_color_at_max_phase);
RUN_TEST(test_breathing_darkens_at_zero_phase);
```

- [ ] **Step 2: 失敗を確認する**

Run:
```bash
nix develop -c pio test -e native
```

Expected: `fillBreathing` 未定義でリンク失敗する。

- [ ] **Step 3: `src/patterns.h` に宣言を追加する**

```cpp
void fillBreathing(Color *leds, size_t n, Color base, uint8_t phase);
```

- [ ] **Step 4: `src/patterns.cpp` に実装を追加する**

```cpp
void fillBreathing(Color *leds, size_t n, Color base, uint8_t phase) {
  uint16_t scale = phase;
  for (size_t i = 0; i < n; ++i) {
    leds[i].r = (uint8_t)((base.r * scale) / 255);
    leds[i].g = (uint8_t)((base.g * scale) / 255);
    leds[i].b = (uint8_t)((base.b * scale) / 255);
  }
}
```

- [ ] **Step 5: テストが通ることを確認する**

Run:
```bash
nix develop -c pio test -e native
```

Expected: breathing テスト 2 件が PASS する。

- [ ] **Step 6: コミットする**

```bash
git add src/patterns.h src/patterns.cpp test/test_patterns/test_main.cpp
git commit -m "feat(patterns): add breathing fill"
```

---

## Task 8: color wipe パターン (TDD)

**Files:**
- Modify: `src/patterns.h`
- Modify: `src/patterns.cpp`
- Modify: `test/test_patterns/test_main.cpp`

- [ ] **Step 1: 失敗するテストを追加する**

```cpp
void test_color_wipe_fills_up_to_progress() {
  const size_t n = 5;
  Color leds[5] = {};
  Color base{0, 200, 0};
  fillColorWipe(leds, n, base, /*progress=*/2);
  TEST_ASSERT_TRUE(colorEquals(leds[0], base));
  TEST_ASSERT_TRUE(colorEquals(leds[1], base));
  TEST_ASSERT_EQUAL_UINT8(0, leds[2].r);
  TEST_ASSERT_EQUAL_UINT8(0, leds[2].g);
  TEST_ASSERT_EQUAL_UINT8(0, leds[2].b);
  TEST_ASSERT_EQUAL_UINT8(0, leds[4].r);
}

void test_color_wipe_clamps_progress_to_n() {
  const size_t n = 3;
  Color leds[3] = {};
  Color base{0, 0, 200};
  fillColorWipe(leds, n, base, /*progress=*/99);
  TEST_ASSERT_TRUE(colorEquals(leds[0], base));
  TEST_ASSERT_TRUE(colorEquals(leds[1], base));
  TEST_ASSERT_TRUE(colorEquals(leds[2], base));
}
```

`main` 内の `RUN_TEST` に追加:

```cpp
RUN_TEST(test_color_wipe_fills_up_to_progress);
RUN_TEST(test_color_wipe_clamps_progress_to_n);
```

- [ ] **Step 2: 失敗を確認する**

Run:
```bash
nix develop -c pio test -e native
```

Expected: `fillColorWipe` 未定義でリンク失敗する。

- [ ] **Step 3: `src/patterns.h` に宣言を追加する**

```cpp
void fillColorWipe(Color *leds, size_t n, Color base, size_t progress);
```

- [ ] **Step 4: `src/patterns.cpp` に実装を追加する**

```cpp
void fillColorWipe(Color *leds, size_t n, Color base, size_t progress) {
  if (n == 0) return;
  size_t upTo = progress;
  if (upTo > n) upTo = n;
  for (size_t i = 0; i < upTo; ++i) {
    leds[i] = base;
  }
  for (size_t i = upTo; i < n; ++i) {
    leds[i] = Color{0, 0, 0};
  }
}
```

- [ ] **Step 5: テストが通ることを確認する**

Run:
```bash
nix develop -c pio test -e native
```

Expected: color wipe テスト 2 件が PASS する。

- [ ] **Step 6: コミットする**

```bash
git add src/patterns.h src/patterns.cpp test/test_patterns/test_main.cpp
git commit -m "feat(patterns): add color wipe fill"
```

---

## Task 9: メインスケッチを配線する

**Files:**
- Modify: `src/main.cpp`

- [ ] **Step 1: `src/main.cpp` を本実装に置き換える**

`/src/main.cpp`:

```cpp
#include <Arduino.h>
#include <FastLED.h>
#include "Color.h"
#include "patterns.h"

namespace {

constexpr uint8_t LED_PIN = 2;
constexpr uint16_t NUM_LEDS = 30;
constexpr uint8_t BRIGHTNESS = 32;

CRgb leds[NUM_LEDS];

uint8_t scaleBrightness(uint8_t v) {
  return (uint8_t)((v * BRIGHTNESS) / 255);
}

void showPattern(uint8_t patternIndex, uint32_t phase) {
  FastLED.clear();
  switch (patternIndex) {
    case 0: {
      uint8_t startHue = (uint8_t)(phase & 0xFF);
      for (uint16_t i = 0; i < NUM_LEDS; ++i) {
        uint8_t hue = startHue + (uint8_t)((i * 256) / NUM_LEDS);
        leds[i] = CRgb(scaleBrightness(hsvToRgb(hue, 255, 255).r),
                       scaleBrightness(hsvToRgb(hue, 255, 255).g),
                       scaleBrightness(hsvToRgb(hue, 255, 255).b));
      }
      break;
    }
    case 1: {
      Color base{200, 0, 0};
      uint8_t breath = (uint8_t)(abs((int)(phase & 0xFF) - 128) * 2);
      for (uint16_t i = 0; i < NUM_LEDS; ++i) {
        Color c{0, 0, 0};
        fillBreathing(&c, 1, base, breath);
        leds[i] = CRgb(scaleBrightness(c.r), scaleBrightness(c.g), scaleBrightness(c.b));
      }
      break;
    }
    case 2: {
      Color base{0, 0, 200};
      uint16_t progress = (uint16_t)((phase / 4) % (NUM_LEDS + 1));
      for (uint16_t i = 0; i < NUM_LEDS; ++i) {
        Color c{0, 0, 0};
        fillColorWipe(&c, 1, base, progress);
        leds[i] = CRgb(scaleBrightness(c.r), scaleBrightness(c.g), scaleBrightness(c.b));
      }
      break;
    }
  }
  FastLED.show();
}

}  // namespace

void setup() {
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.clear();
  FastLED.show();
}

void loop() {
  static uint32_t lastSwitch = 0;
  static uint8_t patternIndex = 0;
  static uint32_t phase = 0;

  constexpr uint32_t patternDurationMs = 5000;
  uint32_t now = millis();
  if (now - lastSwitch >= patternDurationMs) {
    lastSwitch = now;
    patternIndex = (patternIndex + 1) % 3;
    phase = 0;
  }

  showPattern(patternIndex, phase);
  phase += 16;
  delay(16);
}
```

注: `LED_PIN` の `2` は XIAO ESP32C3 の D0 相当 (`GPIO2`)。購入時のシルク印刷と Arduino のピン番号定義に従う。必要なら `D0` 物理シルク側に応じて `LED_PIN` のみ変更する。

- [ ] **Step 2: 実機向けビルドが通ることを確認する**

Run:
```bash
nix develop -c pio run -e esp32c3
```

Expected: `SUCCESS` で終了する。

- [ ] **Step 3: ホスト単体テストが引き続き通ることを確認する**

Run:
```bash
nix develop -c pio test -e native
```

Expected: 全テストが PASS する (`Color` / `hsvToRgb` / `fillRainbow` / `fillBreathing` / `fillColorWipe`)。

- [ ] **Step 4: コミットする**

```bash
git add src/main.cpp
git commit -m "feat(firmware): wire patterns into setup/loop"
```

---

## Task 10: README を書く

**Files:**
- Create: `README.md`

- [ ] **Step 1: `README.md` を作成する**

`/README.md`:

```markdown
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

## 色反転時の対処

WS2812 は `GRB` 順で送出する実装が多いが、稀に赤/緑が入れ替わって見える場合がある。
その場合は `src/main.cpp` の `FastLED.addLeds<WS2812, LED_PIN, GRB>` を `RGB` に変更する。

## 安全上の注意 (PDF 由来)

- LED 1 個あたりの絶対最大は 5V を超えない
- ロール状に巻いたまま点灯しない
- 全白 × 高輝度の長時間点灯は避ける
- 5m を超える長尺は電源注入を検討
- USB 給電だけで多数 LED を駆動しない
```

- [ ] **Step 2: README をコミットする**

```bash
git add README.md
git commit -m "docs: add wiring, build, and safety notes"
```

---

## Task 11: flake の最終チェック

**Files:**
- 変更なし

- [ ] **Step 1: flake の整合性とビルド・テストをまとめて確認する**

Run:
```bash
nix flake check
nix develop -c pio run -e esp32c3
nix develop -c pio test -e native
```

Expected: 3 コマンドすべてが成功する。

- [ ] **Step 2: 完了報告のみ。コミットは不要 (変更なし)**

---

## セルフレビュー

- 仕様カバレッジ:
  - Nix devShell: Task 1
  - PlatformIO 設定: Task 2
  - Arduino + FastLED ファームウェア: Task 5–9
  - README (配線 / ビルド / 書き込み / 安全注意): Task 10
  - 検証: Task 11
  - 仕様で残した曖昧さ (`LED_PIN` 初期 GPIO 番号 / パターン関数名): Task 9 で `LED_PIN = 2` を確定、関数名は `fillRainbow` / `fillBreathing` / `fillColorWipe` で確定
- プレースホルダ: なし (各ステップに実コード / 実コマンドを記載)
- 型整合性: `Color` 構造体は `Color.h` で定義し、`patterns.h` / `patterns.cpp` / `main.cpp` / テストのすべてで同じシグネチャ
- コミット粒度: 1 Task = 1 コミット
