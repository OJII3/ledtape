#include <unity.h>
#include "Color.h"
#include "patterns.h"

void setUp() {}
void tearDown() {}

void test_color_equals_works() {
  Color a{10, 20, 30};
  Color b{10, 20, 30};
  Color c{10, 20, 31};
  TEST_ASSERT_TRUE(colorEquals(a, b));
  TEST_ASSERT_FALSE(colorEquals(a, c));
}

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

void test_rainbow_distributes_hue_across_leds() {
  const size_t n = 4;
  Color leds[4] = {};
  fillRainbow(leds, n, /*startHue=*/0, /*brightness=*/255);
  Color h0 = hsvToRgb(0, 255, 255);
  Color h64 = hsvToRgb(64, 255, 255);
  Color h128 = hsvToRgb(128, 255, 255);
  Color h192 = hsvToRgb(192, 255, 255);
  TEST_ASSERT_TRUE(colorEquals(leds[0], h0));
  TEST_ASSERT_TRUE(colorEquals(leds[1], h64));
  TEST_ASSERT_TRUE(colorEquals(leds[2], h128));
  TEST_ASSERT_TRUE(colorEquals(leds[3], h192));
}

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

int main(int argc, char **argv) {
  UNITY_BEGIN();
  RUN_TEST(test_color_equals_works);
  RUN_TEST(test_hsv_to_rgb_red);
  RUN_TEST(test_hsv_to_rgb_green);
  RUN_TEST(test_hsv_to_rgb_blue);
  RUN_TEST(test_rainbow_distributes_hue_across_leds);
  RUN_TEST(test_breathing_uses_supplied_color_at_max_phase);
  RUN_TEST(test_breathing_darkens_at_zero_phase);
  RUN_TEST(test_color_wipe_fills_up_to_progress);
  RUN_TEST(test_color_wipe_clamps_progress_to_n);
  return UNITY_END();
}
