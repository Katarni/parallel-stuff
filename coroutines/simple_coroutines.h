#pragma once

#include <coroutine>
#include <exception>
#include <iterator>
#include <utility>

namespace kat_coro {
template <typename T>
class Generator {
public:
  struct promise_type {
    T value;

    Generator get_return_object() { return coro_handle_t::from_promise(*this); }
    auto initial_suspend() noexcept { return std::suspend_always{}; }
    auto final_suspend() noexcept { return std::suspend_always{}; }
    void return_void() noexcept {}
    void unhandled_exception() { std::terminate(); }
    auto yield_value(const T& value) {
      this->value = value;
      return std::suspend_always{};
    }
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

  T value() const { return handle_.promise().value; }

  bool next() { return handle_ ? (handle_.resume(), !handle_.done()) : false; }

  void swap(Generator& other) noexcept {
    using std::swap;
    swap(handle_, other.handle_);
  }

private:
  coro_handle_t handle_;
};

template <typename T>
class RangeGenerator {
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

  RangeGenerator(coro_handle_t handle) : handle_(handle) {}
  RangeGenerator(const RangeGenerator& other) = delete;
  RangeGenerator(RangeGenerator&& other) noexcept : handle_(other.handle_) { other.handle_ = nullptr; }
  ~RangeGenerator() {
    if (handle_) {
      handle_.destroy();
    }
  }

  RangeGenerator& operator=(const RangeGenerator& other) = delete;
  RangeGenerator& operator=(RangeGenerator&& other) noexcept {
    RangeGenerator copy(std::move(other));
    swap(copy);
    return *this;
  }

  void swap(RangeGenerator& other) noexcept {
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
