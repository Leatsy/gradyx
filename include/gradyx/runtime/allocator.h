#pragma once

#include <cstddef>

#include "gradyx/core/data_ptr.h"
#include "gradyx/core/result.h"

namespace gradyx {

class Allocator {
 public:
  virtual ~Allocator() = default;

  [[nodiscard]] virtual Result<DataPtr> allocate(std::size_t bytes,
                                                 std::size_t alignment,
                                                 Device device) = 0;
};

}  // namespace gradyx
