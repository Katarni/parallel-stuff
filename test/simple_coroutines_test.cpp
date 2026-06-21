#include "simple_coroutines.h"

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

kat_coro::RangeGenerator<int> range(int begin, const int end, const int step) {
  for (; begin < end; begin += step) {
    co_yield begin;
  }
}

TEST(GeneratorTest, FibonacciTest) {
  auto fib_num = fibonacci();
  std::size_t a = 0;
  std::size_t b = 1;
  fib_num.next();

  for (std::size_t i = 0; i < 15; ++i) {
    EXPECT_EQ(fib_num.value(), a + b);
    a = std::exchange(b, a + b);
    fib_num.next();
  }
}

TEST(GeneratorTest, RangeGenTest) {
  auto j = 1;
  for (auto i : range(1, 10, 2)) {
    EXPECT_EQ(i, j);
    j += 2;
  }
}
