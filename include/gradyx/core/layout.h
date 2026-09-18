#pragma once

#include <cstdint>
#include <iosfwd>
#include <string_view>

#include "gradyx/core/result.h"

namespace gradyx {

enum class Layout : std::uint8_t {
  kStrided = 0,
};

[[nodiscard]] constexpr bool is_valid_layout(Layout layout) noexcept {
  return layout == Layout::kStrided;
}

[[nodiscard]] std::string_view layout_name(Layout layout) noexcept;
[[nodiscard]] Result<Layout> parse_layout(std::string_view text);

std::ostream& operator<<(std::ostream& output, Layout layout);

}  // namespace gradyx
