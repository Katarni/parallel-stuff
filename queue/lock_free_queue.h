#pragma once

#include <atomic>
#include <bits/this_thread_sleep.h>
#include <thread>
#include <vector>

namespace kat_prl {
template <typename T>
class LockFreeQueue {
private:
  struct Node {
    T data;
    std::atomic<std::size_t> id_num;
  };

  std::atomic<std::size_t> head_pos_;
  std::atomic<std::size_t> tail_pos_;
  std::atomic_flag is_done_;
  std::vector<Node> buf_;

public:
  LockFreeQueue() : LockFreeQueue(2048) {}
  explicit LockFreeQueue(std::size_t capacity) : buf_(capacity) {
    head_pos_.store(0, std::memory_order_relaxed);
    tail_pos_.store(0, std::memory_order_relaxed);
    for (std::size_t i = 0; i < capacity; ++i) {
      buf_[i].id_num.store(i, std::memory_order_relaxed);
    }
  }

  [[nodiscard]]
  std::size_t capacity() const noexcept {
    return buf_.size();
  }

  [[nodiscard]]
  bool done() const noexcept {
    return is_done_.test();
  }

  void push(T data) {
    std::size_t pos;
    Node* elm = nullptr;
    bool cas_res = false;

    while (!cas_res) {
      pos = tail_pos_.load();
      elm = std::addressof(buf_[pos % capacity()]);
      auto elm_id = elm->id_num.load();

      if (elm_id < pos) {
        std::this_thread::yield();
      }
      if (elm_id != pos) {
        continue;
      }
      cas_res = tail_pos_.compare_exchange_weak(pos, pos + 1);
    }
    elm->data = std::move(data);
    elm->id_num.store(pos + 1);
  }

  bool pop(T& data) {
    std::size_t pos;
    Node* elm = nullptr;
    bool cas_res = false;

    while (!cas_res) {
      pos = head_pos_.load();
      elm = std::addressof(buf_[pos % capacity()]);
      auto elm_id = elm->id_num.load();

      auto rel_pos = pos + 1;
      if (elm_id < rel_pos) {
        return false;
      }
      if (elm_id != rel_pos) {
        continue;
      }
      cas_res = head_pos_.compare_exchange_weak(pos, pos + 1);
    }
    data = std::move(elm->data);
    elm->id_num.store(pos + capacity());
    return true;
  }

  void set_done() {
    is_done_.test_and_set();
  }
};
} // namespace kat_prl