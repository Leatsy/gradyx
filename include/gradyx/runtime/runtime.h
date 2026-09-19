#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>

#include "gradyx/core/device.h"
#include "gradyx/core/result.h"
#include "gradyx/core/status.h"
#include "gradyx/runtime/allocator.h"
#include "gradyx/runtime/event.h"
#include "gradyx/runtime/stream.h"

namespace gradyx {

struct RuntimeCapabilities final {
  bool asynchronous_execution{false};
  bool pinned_memory{false};
  std::size_t minimum_alignment{alignof(std::max_align_t)};
};

class Runtime {
 public:
  virtual ~Runtime() = default;

  [[nodiscard]] virtual DeviceType device_type() const noexcept = 0;
  [[nodiscard]] virtual Result<std::int32_t> device_count() const = 0;
  [[nodiscard]] virtual Status validate_device(Device device) const = 0;

  [[nodiscard]] virtual Allocator& allocator(Device device) = 0;
  [[nodiscard]] virtual Result<Stream> default_stream(Device device) = 0;
  [[nodiscard]] virtual Result<Stream> create_stream(Device device,
                                                     const StreamOptions& options) = 0;
  [[nodiscard]] virtual Result<std::unique_ptr<Event>> create_event(
      Device device, const EventOptions& options) = 0;

  [[nodiscard]] virtual Status synchronize(Device device) = 0;
  [[nodiscard]] virtual RuntimeCapabilities capabilities(Device device) const = 0;
};

}  // namespace gradyx
