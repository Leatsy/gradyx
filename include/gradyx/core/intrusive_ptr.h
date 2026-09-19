#pragma once

#include <atomic>
#include <concepts>
#include <cstddef>
#include <exception>
#include <limits>
#include <type_traits>
#include <utility>

namespace gradyx {

template <typename T>
class IntrusivePtr;

template <typename T>
class WeakIntrusivePtr;

class IntrusivePtrTarget;

namespace detail {

struct IntrusiveControlBlock final {
  std::atomic<std::size_t> strong_refs{1};
  // The implicit weak reference keeps this block alive while the target lives.
  std::atomic<std::size_t> weak_refs{1};
  std::atomic<IntrusivePtrTarget*> target;
};

struct AdoptIntrusiveRef final {};

inline constexpr AdoptIntrusiveRef kAdoptIntrusiveRef{};

}  // namespace detail

class IntrusivePtrTarget {
 public:
  [[nodiscard]] std::size_t use_count() const noexcept {
    return control_->strong_refs.load(std::memory_order_acquire);
  }

  IntrusivePtrTarget(const IntrusivePtrTarget&) = delete;
  IntrusivePtrTarget& operator=(const IntrusivePtrTarget&) = delete;
  IntrusivePtrTarget(IntrusivePtrTarget&&) = delete;
  IntrusivePtrTarget& operator=(IntrusivePtrTarget&&) = delete;

 protected:
  IntrusivePtrTarget() : control_(new detail::IntrusiveControlBlock{1, 1, this}) {}
  virtual ~IntrusivePtrTarget() noexcept { release_weak_ref(control_); }

 private:
  template <typename T>
  friend class IntrusivePtr;
  template <typename T>
  friend class WeakIntrusivePtr;
  template <typename T, typename... Args>
  friend IntrusivePtr<T> make_intrusive(Args&&... args);

  static void retain_strong_ref(detail::IntrusiveControlBlock* control) {
    increment_ref_count(control->strong_refs);
  }

  static void release_strong_ref(detail::IntrusiveControlBlock* control) noexcept {
    if (control->strong_refs.fetch_sub(1, std::memory_order_acq_rel) == 1) {
      IntrusivePtrTarget* const target =
          control->target.exchange(nullptr, std::memory_order_acq_rel);
      delete target;
    }
  }

  static void retain_weak_ref(detail::IntrusiveControlBlock* control) {
    increment_ref_count(control->weak_refs);
  }

  static void release_weak_ref(detail::IntrusiveControlBlock* control) noexcept {
    if (control->weak_refs.fetch_sub(1, std::memory_order_acq_rel) == 1) {
      delete control;
    }
  }

  [[nodiscard]] static bool try_retain_strong_ref(
      detail::IntrusiveControlBlock* control) noexcept {
    std::size_t count = control->strong_refs.load(std::memory_order_acquire);
    while (count != 0) {
      if (control->strong_refs.compare_exchange_weak(
              count, count + 1, std::memory_order_acq_rel, std::memory_order_acquire)) {
        return true;
      }
    }
    return false;
  }

  static void increment_ref_count(std::atomic<std::size_t>& count) {
    std::size_t current = count.load(std::memory_order_relaxed);
    while (true) {
      if (current == std::numeric_limits<std::size_t>::max()) {
        std::terminate();
      }
      if (count.compare_exchange_weak(current, current + 1, std::memory_order_relaxed,
                                      std::memory_order_relaxed)) {
        return;
      }
    }
  }

  detail::IntrusiveControlBlock* control_;
};

template <typename T>
class IntrusivePtr final {
  static_assert(std::derived_from<T, IntrusivePtrTarget>);

 public:
  constexpr IntrusivePtr() noexcept = default;
  constexpr IntrusivePtr(std::nullptr_t) noexcept {}

  IntrusivePtr(const IntrusivePtr& other) noexcept : target_(other.target_) {
    if (target_ != nullptr) {
      IntrusivePtrTarget::retain_strong_ref(target_->control_);
    }
  }

  IntrusivePtr(IntrusivePtr&& other) noexcept
      : target_(std::exchange(other.target_, nullptr)) {}

  ~IntrusivePtr() { reset(); }

  IntrusivePtr& operator=(const IntrusivePtr& other) noexcept {
    if (this != &other) {
      IntrusivePtr copy{other};
      swap(copy);
    }
    return *this;
  }

  IntrusivePtr& operator=(IntrusivePtr&& other) noexcept {
    if (this != &other) {
      reset();
      target_ = std::exchange(other.target_, nullptr);
    }
    return *this;
  }

  [[nodiscard]] T* get() const noexcept { return target_; }
  [[nodiscard]] T& operator*() const noexcept { return *target_; }
  [[nodiscard]] T* operator->() const noexcept { return target_; }
  [[nodiscard]] explicit operator bool() const noexcept { return target_ != nullptr; }
  [[nodiscard]] std::size_t use_count() const noexcept {
    return target_ == nullptr ? 0 : target_->use_count();
  }

  void reset() noexcept {
    if (T* const target = std::exchange(target_, nullptr); target != nullptr) {
      IntrusivePtrTarget::release_strong_ref(target->control_);
    }
  }

  void swap(IntrusivePtr& other) noexcept { std::swap(target_, other.target_); }

 private:
  template <typename U>
  friend class WeakIntrusivePtr;
  template <typename U, typename... Args>
  friend IntrusivePtr<U> make_intrusive(Args&&... args);

  explicit IntrusivePtr(T* target, detail::AdoptIntrusiveRef) noexcept
      : target_(target) {}

  T* target_{nullptr};
};

template <typename T>
class WeakIntrusivePtr final {
  static_assert(std::derived_from<T, IntrusivePtrTarget>);

 public:
  constexpr WeakIntrusivePtr() noexcept = default;

  WeakIntrusivePtr(const IntrusivePtr<T>& strong) noexcept
      : control_(strong.target_ == nullptr ? nullptr : strong.target_->control_) {
    if (control_ != nullptr) {
      IntrusivePtrTarget::retain_weak_ref(control_);
    }
  }

  WeakIntrusivePtr(const WeakIntrusivePtr& other) noexcept : control_(other.control_) {
    if (control_ != nullptr) {
      IntrusivePtrTarget::retain_weak_ref(control_);
    }
  }

  WeakIntrusivePtr(WeakIntrusivePtr&& other) noexcept
      : control_(std::exchange(other.control_, nullptr)) {}

  ~WeakIntrusivePtr() { reset(); }

  WeakIntrusivePtr& operator=(const WeakIntrusivePtr& other) noexcept {
    if (this != &other) {
      WeakIntrusivePtr copy{other};
      swap(copy);
    }
    return *this;
  }

  WeakIntrusivePtr& operator=(WeakIntrusivePtr&& other) noexcept {
    if (this != &other) {
      reset();
      control_ = std::exchange(other.control_, nullptr);
    }
    return *this;
  }

  [[nodiscard]] IntrusivePtr<T> lock() const noexcept {
    if (control_ == nullptr || !IntrusivePtrTarget::try_retain_strong_ref(control_)) {
      return {};
    }
    return IntrusivePtr<T>(
        static_cast<T*>(control_->target.load(std::memory_order_acquire)),
        detail::kAdoptIntrusiveRef);
  }

  [[nodiscard]] bool expired() const noexcept {
    return control_ == nullptr ||
           control_->strong_refs.load(std::memory_order_acquire) == 0;
  }

  void reset() noexcept {
    if (auto* const control = std::exchange(control_, nullptr); control != nullptr) {
      IntrusivePtrTarget::release_weak_ref(control);
    }
  }

  void swap(WeakIntrusivePtr& other) noexcept { std::swap(control_, other.control_); }

 private:
  detail::IntrusiveControlBlock* control_{nullptr};
};

template <typename T, typename... Args>
[[nodiscard]] IntrusivePtr<T> make_intrusive(Args&&... args) {
  static_assert(std::derived_from<T, IntrusivePtrTarget>);
  return IntrusivePtr<T>(new T(std::forward<Args>(args)...),
                         detail::kAdoptIntrusiveRef);
}

template <typename T>
void swap(IntrusivePtr<T>& lhs, IntrusivePtr<T>& rhs) noexcept {
  lhs.swap(rhs);
}

template <typename T>
void swap(WeakIntrusivePtr<T>& lhs, WeakIntrusivePtr<T>& rhs) noexcept {
  lhs.swap(rhs);
}

}  // namespace gradyx
