#include "gradyx/core/layout.h"

#include <ostream>
#include <string>

namespace gradyx {

std::string_view layout_name(Layout layout) noexcept {
  if (layout == Layout::kStrided) {
    return "strided";
  }
  return "invalid";
}

Result<Layout> parse_layout(std::string_view text) {
  if (text == "strided") {
    return Layout::kStrided;
  }
  return Status::invalid_argument("unknown layout: " + std::string{text});
}

std::ostream& operator<<(std::ostream& output, Layout layout) {
  if (is_valid_layout(layout)) {
    return output << layout_name(layout);
  }
  return output << "invalid_layout(" << static_cast<unsigned int>(layout) << ')';
}

}  // namespace gradyx
