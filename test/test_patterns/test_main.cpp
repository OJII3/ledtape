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

int main(int argc, char **argv) {
  UNITY_BEGIN();
  RUN_TEST(test_color_equals_works);
  RUN_TEST(test_hsv_to_rgb_red);
  RUN_TEST(test_hsv_to_rgb_green);
  RUN_TEST(test_hsv_to_rgb_blue);
  return UNITY_END();
}
