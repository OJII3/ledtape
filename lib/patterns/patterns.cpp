#include "patterns.h"

Color hsvToRgb(uint8_t hue, uint8_t sat, uint8_t val) {
  uint8_t region = hue / 85;
  uint8_t rem = hue % 85;
  uint8_t f = rem * 255 / 84;

  uint8_t p = (val * (255 - sat)) / 255;
  uint8_t t = (val * (255 - ((uint16_t)sat * (255 - f) / 255))) / 255;

  switch (region) {
    case 0: return Color{val, t, p};
    case 1: return Color{p, val, t};
    default: return Color{t, p, val};
  }
}

void fillRainbow(Color *leds, size_t n, uint8_t startHue, uint8_t brightness) {
  if (n == 0) return;
  for (size_t i = 0; i < n; ++i) {
    uint8_t hue = startHue + (uint8_t)((i * 256) / n);
    leds[i] = hsvToRgb(hue, 255, brightness);
  }
}

void fillBreathing(Color *leds, size_t n, Color base, uint8_t phase) {
  uint16_t scale = phase;
  for (size_t i = 0; i < n; ++i) {
    leds[i].r = (uint8_t)((base.r * scale) / 255);
    leds[i].g = (uint8_t)((base.g * scale) / 255);
    leds[i].b = (uint8_t)((base.b * scale) / 255);
  }
}
