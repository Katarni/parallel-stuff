#include "generators.h"

#include <cstddef>
#include <gtest/gtest.h>

kat_coro::Generator<std::size_t> fibonacci() {
  std::size_t a = 0;
  std::size_t b = 1;

  for (;;) {
    a = std::exchange(b, a + b);
    co_yield b;
  }
}

kat_coro::Generator<std::size_t> nat_nums() {
  for (std::size_t num = 0;; ++num) {
    co_yield num;
  }
}

kat_coro::ViewGenerator<int> range(int begin, const int end, const int step) {
  for (; begin < end; begin += step) {
    co_yield begin;
  }
}

TEST(GeneratorTest, FibonacciTest) {
  auto fib_num = fibonacci();
  std::size_t a = 0;
  std::size_t b = 1;
  fib_num.resume();

  for (std::size_t i = 0; i < 15; ++i) {
    EXPECT_EQ(fib_num.value(), a + b);
    a = std::exchange(b, a + b);
    fib_num.resume();
  }
}

TEST(GeneratorTest, SkipTest) {
  auto num = nat_nums();
  num.resume();
  EXPECT_EQ(num.value(), 0);
  num.resume();
  EXPECT_EQ(num.value(), 1);
  num.skip(2);
  EXPECT_EQ(num.value(), 4);
}

TEST(GeneratorTest, ViewGenTest) {
  auto j = 1;
  for (auto i : range(1, 10, 2)) {
    EXPECT_EQ(i, j);
    j += 2;
  }
}
