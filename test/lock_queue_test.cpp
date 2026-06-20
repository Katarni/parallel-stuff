#include "lock_queue.h"

#include <gtest/gtest.h>
#include <vector>

constexpr std::size_t kTaskNum = 1000;

int task_cnt;
std::mutex produce_tasks_mtx;

std::vector<int> consumed_tasks;
std::mutex consumed_tasks_mtx;

void produce(kat_prl::lock_queue<int>& queue) {
  for (;;) {
    int num;

    {
      std::lock_guard lock{produce_tasks_mtx};
      if (task_cnt <= 0) {
        break;
      }
      num = task_cnt--;
    }

    queue.push(num);
  }
}

void consume(kat_prl::lock_queue<int>& queue) {
  for (;;) {
    int num;

    if (!queue.pop(num)) {
      break;
    }

    std::lock_guard lock{consumed_tasks_mtx};
    consumed_tasks.push_back(num);
  }
}

class LockQueueTest : public ::testing::TestWithParam<std::tuple<std::size_t, std::size_t>> {};

TEST_P(LockQueueTest, SingleTest) {
  task_cnt = kTaskNum;
  consumed_tasks.clear();

  auto [prods_cnt, cons_cnt] = GetParam();

  std::vector<std::thread> prods;
  std::vector<std::thread> cons;
  kat_prl::lock_queue<int> queue;

  for (std::size_t i = 0; i < prods_cnt; ++i) {
    prods.emplace_back(produce, std::ref(queue));
  }

  for (std::size_t i = 0; i < cons_cnt; ++i) {
    cons.emplace_back(consume, std::ref(queue));
  }

  for (auto& t : prods) {
    t.join();
  }
  queue.set_done();

  for (auto& t : cons) {
    t.join();
  }

  ASSERT_EQ(task_cnt, 0);
  ASSERT_EQ(consumed_tasks.size(), kTaskNum);
}

INSTANTIATE_TEST_SUITE_P(
    LockQueueTest,
    LockQueueTest,
    ::testing::Combine(
        ::testing::Values(1, 2, 4),
        ::testing::Values(1, 2, 4)
    )
);
