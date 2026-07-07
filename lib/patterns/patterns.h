#pragma once

#include "Color.h"

Color hsvToRgb(uint8_t hue, uint8_t sat, uint8_t val);
void fillRainbow(Color *leds, size_t n, uint8_t startHue, uint8_t brightness);
void fillBreathing(Color *leds, size_t n, Color base, uint8_t phase);
