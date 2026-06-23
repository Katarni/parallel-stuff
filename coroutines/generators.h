#pragma once

#include <coroutine>
#include <exception>
#include <iterator>
#include <utility>

namespace kat_coro {
template <typename T>
class Generator {
public:
  struct awaitable {
    bool ready{true};

    bool await_ready() const noexcept { return ready; }
    void await_suspend(std::coroutine_handle<>) const noexcept {}
    void await_resume() const noexcept {}
  };

  struct promise_type {
    friend Generator;

    Generator get_return_object() { return coro_handle_t::from_promise(*this); }
    auto initial_suspend() noexcept { return std::suspend_always{}; }
    auto final_suspend() noexcept { return std::suspend_always{}; }
    void return_void() noexcept {}
    void unhandled_exception() { std::terminate(); }
    awaitable yield_value(const T& value) {
      value_ = value;
      return awaitable{static_cast<bool>(skip_cnt_ ? skip_cnt_-- : skip_cnt_)};
    }

    T value() const { return value_; }

  private:
    std::size_t skip_cnt_{0};
    T value_;
  };

  using coro_handle_t = std::coroutine_handle<promise_type>;

  Generator(const coro_handle_t handle) : handle_(handle) {}
  Generator(const Generator& other) = delete;
  Generator(Generator&& other) noexcept : handle_(other.handle_) { other.handle_ = nullptr; }
  ~Generator() {
    if (handle_) {
      handle_.destroy();
    }
  }

  Generator& operator=(const Generator& other) = delete;
  Generator& operator=(Generator&& other) noexcept {
    Generator copy(std::move(other));
    swap(copy);
    return *this;
  }

  T value() const { return handle_.promise().value(); }

  bool resume() { return handle_ ? (handle_.resume(), !handle_.done()) : false; }
  bool skip(std::size_t cnt) {
    handle_.promise().skip_cnt_ = cnt;
    return resume();
  }

  void swap(Generator& other) noexcept {
    using std::swap;
    swap(handle_, other.handle_);
  }

private:
  coro_handle_t handle_;
};

template <typename T>
class ViewGenerator {
public:
  struct promise_type {
    const T* value;

    auto get_return_object() { return coro_handle_t::from_promise(*this); }
    auto initial_suspend() noexcept { return std::suspend_always{}; }
    auto final_suspend() noexcept { return std::suspend_always{}; }
    void return_void() {}
    void unhandled_exception() { std::terminate(); }
    auto yield_value(const T& value) {
      this->value = std::addressof(value);
      return std::suspend_always{};
    }
  };

  using coro_handle_t = std::coroutine_handle<promise_type>;

  struct iterator {
    using iterator_category = std::forward_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value = T;
    using pointer = const T*;
    using reference = const T&;

    iterator(coro_handle_t handle) : handle_(handle) {}

    iterator& operator++() {
      handle_.resume();
      if (handle_.done()) {
        handle_ = nullptr;
      }
      return *this;
    }

    reference operator*() const { return *handle_.promise().value; }
    pointer operator->() const { return handle_.promise().value; }
    auto operator<=>(const iterator&) const noexcept = default;

  private:
    coro_handle_t handle_;
  };

  ViewGenerator(coro_handle_t handle) : handle_(handle) {}
  ViewGenerator(const ViewGenerator& other) = delete;
  ViewGenerator(ViewGenerator&& other) noexcept : handle_(other.handle_) { other.handle_ = nullptr; }
  ~ViewGenerator() {
    if (handle_) {
      handle_.destroy();
    }
  }

  ViewGenerator& operator=(const ViewGenerator& other) = delete;
  ViewGenerator& operator=(ViewGenerator&& other) noexcept {
    ViewGenerator copy(std::move(other));
    swap(copy);
    return *this;
  }

  void swap(ViewGenerator& other) noexcept {
    using std::swap;
    swap(handle_, other.handle_);
  }

  iterator begin() {
    if (!handle_) {
      return {nullptr};
    }

    handle_.resume();
    if (handle_.done()) {
      return {nullptr};
    }
    return {handle_};
  }

  iterator end() { return {nullptr}; }

private:
  coro_handle_t handle_;
};
} // namespace kat_coro
