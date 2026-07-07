#include <Arduino.h>
#include <FastLED.h>
#include "Color.h"
#include "patterns.h"

namespace {

constexpr uint8_t LED_PIN = 2;
constexpr uint16_t NUM_LEDS = 120;
constexpr uint8_t BRIGHTNESS = 32;

CRGB leds[NUM_LEDS];
Color patternLeds[NUM_LEDS];

void renderPattern(uint8_t patternIndex, uint32_t phase) {
  switch (patternIndex) {
    case 0: {
      fillRainbow(patternLeds, NUM_LEDS, (uint8_t)(phase & 0xFF), 255);
      break;
    }
    case 1: {
      Color base{200, 0, 0};
      uint8_t breath = (uint8_t)(abs((int)(phase & 0xFF) - 128) * 2);
      fillBreathing(patternLeds, NUM_LEDS, base, breath);
      break;
    }
    case 2: {
      Color base{0, 0, 200};
      uint16_t progress = (uint16_t)((phase / 4) % (NUM_LEDS + 1));
      fillColorWipe(patternLeds, NUM_LEDS, base, progress);
      break;
    }
  }
  for (uint16_t i = 0; i < NUM_LEDS; ++i) {
    leds[i] = CRGB(patternLeds[i].r, patternLeds[i].g, patternLeds[i].b);
  }
}

}  // namespace

void setup() {
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.clear(true);
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

  renderPattern(patternIndex, phase);
  FastLED.show();
  phase += 16;
  delay(16);
}
