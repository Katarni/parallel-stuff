#include <gtest/gtest.h>
#include "event_subscribe.h"

namespace {
Eventer event;
std::size_t count{0};

EventSubscriber sub1() {
  count += 1;
  co_await event;
  count += 1;
}

EventSubscriber sub2() {
  count += 2;
  co_await event;
  count += 2;
  co_await event;
  count += 2;
}
}

TEST(CoroutinesTest, EventTest) {
  sub1();
  sub2();

  EXPECT_EQ(count, 3);
  event.set();

  sub1();
  EXPECT_EQ(count, 7);
  event.set();

  EXPECT_EQ(count, 10);
  event.set();
  EXPECT_EQ(count, 10);
}