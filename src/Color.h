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
