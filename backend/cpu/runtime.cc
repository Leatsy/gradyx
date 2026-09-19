#include "gradyx/backend/cpu/runtime.h"

#include <algorithm>
#include <cstdlib>
#include <limits>
#include <memory>
#include <mutex>
#include <string>

namespace gradyx {
namespace {

[[nodiscard]] bool is_power_of_two(std::size_t value) noexcept {
  return value != 0 && (value & (value - 1)) == 0;
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
void free_cpu_memory(void* data, void* context) noexcept {
  static_cast<void>(context);
  std::free(data);
}

class CpuEvent final : public Event {
 public:
  explicit CpuEvent(Device device) noexcept : device_(device) {}

  Status record(const Stream& stream) override {
    if (!stream || stream.device() != device_) {
      return Status::invalid_argument(
          "cannot record a CPU event on a stream from another device");
    }

    std::lock_guard lock{mutex_};
    recorded_ = true;
    completed_ = true;
    return {};
  }

  Result<bool> query() const override {
    std::lock_guard lock{mutex_};
    if (!recorded_) {
      return Status::error(ErrorCode::kFailedPrecondition,
                           "cannot query an event before it is recorded");
    }
    return completed_;
  }

  Status synchronize() override {
    std::lock_guard lock{mutex_};
    if (!recorded_) {
      return Status::error(ErrorCode::kFailedPrecondition,
                           "cannot synchronize an event before it is recorded");
    }
    return {};
  }

  Status wait(const Stream& stream) override {
    if (!stream || stream.device() != device_) {
      return Status::invalid_argument(
          "cannot wait for a CPU event on a stream from another device");
    }

    std::lock_guard lock{mutex_};
    if (!recorded_) {
      return Status::error(ErrorCode::kFailedPrecondition,
                           "cannot wait for an event before it is recorded");
    }
    return {};
  }

 private:
  Device device_;
  mutable std::mutex mutex_;
  bool recorded_{false};
  bool completed_{false};
};

}  // namespace

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
Result<DataPtr> CpuAllocator::allocate(std::size_t bytes, std::size_t alignment,
                                       Device device) {
  if (device != Device{DeviceType::kCPU}) {
    return Status::invalid_argument("CPU allocator only accepts cpu:0");
  }
  if (!is_power_of_two(alignment)) {
    return Status::invalid_argument("allocation alignment must be a power of two");
  }
  if (bytes == 0) {
    return DataPtr{};
  }

  const std::size_t actual_alignment = std::max(alignment, alignof(std::max_align_t));
  if (actual_alignment < sizeof(void*)) {
    return Status::invalid_argument("allocation alignment is too small");
  }

  if (bytes > static_cast<std::size_t>(std::numeric_limits<std::ptrdiff_t>::max())) {
    return Status::error(ErrorCode::kResourceExhausted,
                         "CPU allocator cannot allocate " + std::to_string(bytes) +
                             " bytes on " + device.str());
  }
  void* data = nullptr;
  if (posix_memalign(&data, actual_alignment, bytes) != 0) {
    return Status::error(ErrorCode::kResourceExhausted,
                         "CPU allocator cannot allocate " + std::to_string(bytes) +
                             " bytes on " + device.str());
  }
  return DataPtr{{data, bytes, device}, nullptr, free_cpu_memory};
}

DeviceType CpuRuntime::device_type() const noexcept { return DeviceType::kCPU; }

Result<std::int32_t> CpuRuntime::device_count() const { return 1; }

Status CpuRuntime::validate_device(Device device) const {
  if (device != Device{DeviceType::kCPU}) {
    return Status::invalid_argument("CPU runtime only accepts cpu:0");
  }
  return {};
}

Allocator& CpuRuntime::allocator(Device device) {
  static_cast<void>(device);
  return allocator_;
}

Result<Stream> CpuRuntime::default_stream(Device device) {
  if (const Status status = validate_device(device); !status) {
    return status;
  }
  return Stream{device, 0, true};
}

Result<Stream> CpuRuntime::create_stream(Device device, const StreamOptions& options) {
  if (const Status status = validate_device(device); !status) {
    return status;
  }
  static_cast<void>(options);
  return Stream{device, next_stream_id_.fetch_add(1, std::memory_order_relaxed), false};
}

Result<std::unique_ptr<Event>> CpuRuntime::create_event(Device device,
                                                        const EventOptions& options) {
  if (const Status status = validate_device(device); !status) {
    return status;
  }
  static_cast<void>(options);
  std::unique_ptr<Event> event = std::make_unique<CpuEvent>(device);
  return event;
}

Status CpuRuntime::synchronize(Device device) { return validate_device(device); }

RuntimeCapabilities CpuRuntime::capabilities(Device device) const {
  static_cast<void>(device);
  return {
      .asynchronous_execution = false,
      .pinned_memory = false,
      .minimum_alignment = alignof(std::max_align_t),
  };
}

Status register_cpu_runtime(RuntimeRegistry& registry) {
  return registry.register_runtime(std::make_shared<CpuRuntime>());
}

Status register_builtin_runtimes(RuntimeRegistry& registry) {
  return register_cpu_runtime(registry);
}

RuntimeRegistry& default_runtime_registry() {
  static RuntimeRegistry registry;
  static const Status initialization = register_builtin_runtimes(registry);
  static_cast<void>(initialization);
  return registry;
}

}  // namespace gradyx
