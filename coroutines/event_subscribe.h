#pragma once

#include <coroutine>
#include <exception>
#include <queue>

struct EventSubscriber {
  struct promise_type;
  using coro_handle_t = std::coroutine_handle<promise_type>;

  struct promise_type {
    auto get_return_object() { return coro_handle_t::from_promise(*this); }
    auto initial_suspend() noexcept { return std::suspend_never{}; }
    auto final_suspend() noexcept { return std::suspend_never{}; } // self-destroy
    void return_void() {}
    void unhandled_exception() { std::terminate(); }
  };

  EventSubscriber(coro_handle_t) {}
  EventSubscriber(const EventSubscriber&) {}
  EventSubscriber(EventSubscriber&&) noexcept {}

  EventSubscriber& operator=(const EventSubscriber&) { return *this; }
  EventSubscriber& operator=(EventSubscriber&&) noexcept { return *this; }
};

class Eventer {
public:
  using empty_coro_t = std::coroutine_handle<>;
  friend struct awaiter;

  struct awaiter {
    empty_coro_t handle{nullptr};
    Eventer& subscription;

    explicit awaiter(Eventer& event) : subscription(event) {}

    bool await_ready() const noexcept { return subscription.is_ready(); }
    void await_suspend(const empty_coro_t coro) noexcept {
      handle = coro;
      subscription.subscribers_.push(*this);
    }
    void await_resume() const noexcept { subscription.reset(); }
  };

  explicit Eventer(const bool ready = false) : is_ready_(ready) {}
  Eventer(const Eventer&) = delete;
  Eventer(Eventer&&) = delete;
  Eventer& operator=(const Eventer&) = delete;
  Eventer& operator=(Eventer&&) = delete;

  awaiter operator co_await() noexcept {
    return awaiter{*this};
  }

  bool is_ready() const { return is_ready_; }
  void reset() { is_ready_ = false; }
  void set() {
    is_ready_ = true;
    resumeSubscribers();
  }

private:
  bool is_ready_;
  std::queue<awaiter> subscribers_;

  void resumeSubscribers() {
    auto resume_cnt = std::ssize(subscribers_);
    while (resume_cnt--) {
      subscribers_.front().handle.resume();
      subscribers_.pop();
    }
  }
};
