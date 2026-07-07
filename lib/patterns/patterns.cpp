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
