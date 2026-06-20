#pragma once

#include <condition_variable>
#include <iostream>
#include <mutex>
#include <vector>

namespace kat_prl {
template <typename T>
class LockQueue {
private:
  std::mutex mtx_;
  std::vector<T> buf_;
  std::condition_variable push_conditional_;
  std::condition_variable pop_conditional_;
  bool is_done_;
  std::size_t pos_, size_;

  [[nodiscard]]
  std::size_t size() const noexcept {
    return size_;
  }

  [[nodiscard]]
  std::size_t capacity() const noexcept {
    return buf_.size();
  }

  [[nodiscard]]
  bool full() const noexcept {
    return size() == buf_.size();
  }

  [[nodiscard]]
  bool empty() const noexcept {
    return size() == 0;
  }

  [[nodiscard]]
  bool done() const noexcept {
    return is_done_;
  }

public:
  LockQueue() : LockQueue(2048) {}
  explicit LockQueue(std::size_t capacity) : buf_(capacity), is_done_(false), pos_(0), size_(0) {}

  void push(T elm) {
    std::unique_lock lock{mtx_};
    push_conditional_.wait(lock, [this]() { return !full(); });

    const auto new_size = size() + 1;
    buf_[(pos_ + new_size) % capacity()] = elm;
    size_ = new_size;
    pop_conditional_.notify_one();
  }

  bool pop(T& elm) {
    std::unique_lock lock{mtx_};
    pop_conditional_.wait(lock, [this]() { return !empty() || done(); });

    if (empty()) {
      return false;
    }

    elm = buf_[pos_ % capacity()];
    ++pos_;
    --size_;
    push_conditional_.notify_one();
    return true;
  }

  void set_done() {
    std::unique_lock lock{mtx_};
    is_done_ = true;
    pop_conditional_.notify_all();
  }

  bool empty_and_done() {
    std::unique_lock lock{mtx_};
    return done() && empty();
  }
};
} // namespace kat_prl
