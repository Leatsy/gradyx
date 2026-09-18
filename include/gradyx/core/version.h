#pragma once

#include <string_view>

namespace gradyx {

inline constexpr int kVersionMajor = 0;
inline constexpr int kVersionMinor = 0;
inline constexpr int kVersionPatch = 0;

[[nodiscard]] std::string_view version() noexcept;

}  // namespace gradyx
