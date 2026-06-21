#pragma once

#include <atomic>
#include <memory>
#include <thread>

namespace kat_prl {
template <typename T>
class LockFreeStack {
private:
  struct Node {
    std::shared_ptr<Node> next;
    T data;

    explicit Node(T data) : next(nullptr), data(std::move(data)) {}
  };

  std::atomic<std::shared_ptr<Node>> head_{nullptr};
  std::atomic_flag is_done_{};

public:
  [[nodiscard]]
  bool empty() const {
    return head_.load() == nullptr;
  }

  [[nodiscard]]
  bool done() const {
    return is_done_.test();
  }

  void set_done() {
    is_done_.test_and_set();
  }

  void push(T data) {
    auto new_head = std::make_shared<Node>(std::move(data));
    new_head->next = head_.load(std::memory_order_acquire);
    while (!head_.compare_exchange_weak(new_head->next, new_head)) {
      std::this_thread::yield();
    }
  }

  std::shared_ptr<T> pop() {
    auto cur = head_.load(std::memory_order_acquire);
    while (cur != nullptr && !head_.compare_exchange_weak(cur, cur->next)) {
      std::this_thread::yield();
    }

    if (cur == nullptr) {
      return nullptr;
    }

    return std::shared_ptr<T>(cur, &cur->data);
  }
};
} // namespace kat_prl