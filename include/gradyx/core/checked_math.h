#pragma once

#include <concepts>
#include <limits>
#include <type_traits>

#include "gradyx/core/result.h"

namespace gradyx {

template <std::integral T>
  requires(!std::same_as<std::remove_cv_t<T>, bool>)
[[nodiscard]] Result<T> checked_add(T lhs, T rhs) {
  constexpr auto minimum = std::numeric_limits<T>::min();
  constexpr auto maximum = std::numeric_limits<T>::max();

  if constexpr (std::is_unsigned_v<T>) {
    if (lhs > maximum - rhs) {
      return Status::overflow("integer addition overflow");
    }
  } else {
    if ((rhs > 0 && lhs > maximum - rhs) || (rhs < 0 && lhs < minimum - rhs)) {
      return Status::overflow("integer addition overflow");
    }
  }
  return static_cast<T>(lhs + rhs);
}

template <std::integral T>
  requires(!std::same_as<std::remove_cv_t<T>, bool>)
[[nodiscard]] Result<T> checked_multiply(T lhs, T rhs) {
  constexpr auto minimum = std::numeric_limits<T>::min();
  constexpr auto maximum = std::numeric_limits<T>::max();

  if (lhs == 0 || rhs == 0) {
    return T{0};
  }

  if constexpr (std::is_unsigned_v<T>) {
    if (lhs > maximum / rhs) {
      return Status::overflow("integer multiplication overflow");
    }
  } else {
    if ((lhs == -1 && rhs == minimum) || (rhs == -1 && lhs == minimum)) {
      return Status::overflow("integer multiplication overflow");
    }
    if ((lhs > 0 && rhs > 0 && lhs > maximum / rhs) ||
        (lhs > 0 && rhs < 0 && rhs < minimum / lhs) ||
        (lhs < 0 && rhs > 0 && lhs < minimum / rhs) ||
        (lhs < 0 && rhs < 0 && lhs < maximum / rhs)) {
      return Status::overflow("integer multiplication overflow");
    }
  }
  return static_cast<T>(lhs * rhs);
}

}  // namespace gradyx
