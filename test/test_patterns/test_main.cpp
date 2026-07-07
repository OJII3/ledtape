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
