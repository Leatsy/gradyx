#pragma once

#include <stdexcept>
#include <type_traits>
#include <utility>
#include <variant>

#include "gradyx/core/status.h"

namespace gradyx {

template <typename T>
class [[nodiscard]] Result final {
  static_assert(!std::is_same_v<std::remove_cv_t<T>, Status>);
  static_assert(!std::is_reference_v<T>);

 public:
  Result(const T& value) : storage_(value) {}
  Result(T&& value) : storage_(std::move(value)) {}

  Result(Status status) : storage_(normalize_error(std::move(status))) {}

  [[nodiscard]] bool ok() const noexcept { return std::holds_alternative<T>(storage_); }

  [[nodiscard]] explicit operator bool() const noexcept { return ok(); }

  [[nodiscard]] const Status& status() const& noexcept {
    if (const auto* error = std::get_if<Status>(&storage_)) {
      return *error;
    }
    return ok_status();
  }

  [[nodiscard]] T& value() & {
    ensure_value();
    return std::get<T>(storage_);
  }

  [[nodiscard]] const T& value() const& {
    ensure_value();
    return std::get<T>(storage_);
  }

  [[nodiscard]] T&& value() && {
    ensure_value();
    return std::get<T>(std::move(storage_));
  }

  [[nodiscard]] T& operator*() & { return value(); }
  [[nodiscard]] const T& operator*() const& { return value(); }
  [[nodiscard]] T* operator->() { return &value(); }
  [[nodiscard]] const T* operator->() const { return &value(); }

 private:
  [[nodiscard]] static Status normalize_error(Status status) {
    if (status.ok()) {
      return Status::internal("Result cannot be constructed from an OK status");
    }
    return status;
  }

  void ensure_value() const {
    if (!ok()) {
      throw std::logic_error(status().to_string());
    }
  }

  std::variant<T, Status> storage_;
};

}  // namespace gradyx
