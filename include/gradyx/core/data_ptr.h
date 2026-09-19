#pragma once

#include <cstddef>
#include <utility>

#include "gradyx/core/device.h"

namespace gradyx {

struct Allocation final {
  void* data{nullptr};
  std::size_t bytes{0};
  Device device{DeviceType::kCPU};
};

class DataPtr final {
 public:
  using Deleter = void (*)(void* data, void* context) noexcept;

  // Device is initialized with a known-valid CPU value; clang-tidy cannot
  // prove that the Device validation path is non-throwing here.
  // NOLINTNEXTLINE(bugprone-exception-escape)
  DataPtr() noexcept = default;
  DataPtr(Allocation allocation, void* context, Deleter deleter) noexcept
      : allocation_(allocation), context_(context), deleter_(deleter) {}

  DataPtr(DataPtr&& other) noexcept
      : allocation_(std::exchange(other.allocation_, {})),
        context_(std::exchange(other.context_, nullptr)),
        deleter_(std::exchange(other.deleter_, nullptr)) {}
  ~DataPtr() { reset(); }

  DataPtr(const DataPtr&) = delete;
  DataPtr& operator=(const DataPtr&) = delete;

  DataPtr& operator=(DataPtr&& other) noexcept {
    if (this != &other) {
      reset();
      allocation_ = std::exchange(other.allocation_, {});
      context_ = std::exchange(other.context_, nullptr);
      deleter_ = std::exchange(other.deleter_, nullptr);
    }
    return *this;
  }

  [[nodiscard]] void* data() const noexcept { return allocation_.data; }
  [[nodiscard]] std::size_t bytes() const noexcept { return allocation_.bytes; }
  [[nodiscard]] Device device() const noexcept { return allocation_.device; }
  [[nodiscard]] explicit operator bool() const noexcept {
    return allocation_.data != nullptr;
  }

  void reset() noexcept {
    if (allocation_.data != nullptr && deleter_ != nullptr) {
      deleter_(allocation_.data, context_);
    }
    allocation_ = {};
    context_ = nullptr;
    deleter_ = nullptr;
  }

  void swap(DataPtr& other) noexcept {
    std::swap(allocation_.data, other.allocation_.data);
    std::swap(allocation_.bytes, other.allocation_.bytes);
    std::swap(allocation_.device, other.allocation_.device);
    std::swap(context_, other.context_);
    std::swap(deleter_, other.deleter_);
  }

 private:
  Allocation allocation_{};
  void* context_{nullptr};
  Deleter deleter_{nullptr};
};

inline void swap(DataPtr& lhs, DataPtr& rhs) noexcept { lhs.swap(rhs); }

}  // namespace gradyx
