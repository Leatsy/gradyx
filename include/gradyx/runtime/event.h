#pragma once

#include "gradyx/core/result.h"
#include "gradyx/core/status.h"
#include "gradyx/runtime/stream.h"

namespace gradyx {

struct EventOptions final {
  bool enable_timing{false};
};

class Event {
 public:
  virtual ~Event() = default;

  [[nodiscard]] virtual Status record(const Stream& stream) = 0;
  [[nodiscard]] virtual Result<bool> query() const = 0;
  [[nodiscard]] virtual Status synchronize() = 0;
  [[nodiscard]] virtual Status wait(const Stream& stream) = 0;
};

}  // namespace gradyx
