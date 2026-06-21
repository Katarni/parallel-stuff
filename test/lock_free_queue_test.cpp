#include "lock_free_queue.h"
#include "test_helper.h"

#include <gtest/gtest.h>
#include <vector>

namespace {
std::atomic<int> task_cnt;

std::mutex consumed_tasks_mtx;
std::vector<int> consumed_tasks;

void produce(kat_prl::LockFreeQueue<int>& queue) {
  for (;;) {
    auto num = task_cnt.load();
    while (num > 0 && !task_cnt.compare_exchange_weak(num, num - 1)) {
      std::this_thread::yield();
    }
    if (num <= 0) {
      return;
    }

    queue.push(num);
  }
}

void consume(kat_prl::LockFreeQueue<int>& queue) {
  for (;;) {
    int data;

    if (!queue.pop(data)) {
      if (queue.done()) {
        break;
      }
      std::this_thread::yield();
      continue;
    }
    std::lock_guard lock{consumed_tasks_mtx};
    consumed_tasks.push_back(data);
  }
}
} // namespace

class LockFreeQueueTest : public ::testing::TestWithParam<std::pair<std::size_t, std::size_t>> {};

TEST_P(LockFreeQueueTest, IntTasksTest) {
  task_cnt = kTaskNum;
  consumed_tasks.clear();

  auto [prods_cnt, cons_cnt] = GetParam();

  std::vector<std::thread> prods;
  std::vector<std::thread> cons;
  kat_prl::LockFreeQueue<int> queue;

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

INSTANTIATE_TEST_SUITE_P(ThreadTest, LockFreeQueueTest, ::testing::ValuesIn(threads_nums));
