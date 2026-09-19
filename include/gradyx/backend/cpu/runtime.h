#pragma once

#include <atomic>
#include <cstddef>
#include <memory>

#include "gradyx/runtime/registry.h"
#include "gradyx/runtime/runtime.h"

namespace gradyx {

class CpuAllocator final : public Allocator {
 public:
  // The Allocator interface intentionally accepts bytes and alignment together.
  // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
  [[nodiscard]] Result<DataPtr> allocate(std::size_t bytes, std::size_t alignment,
                                         Device device) override;
};

class CpuRuntime final : public Runtime {
 public:
  CpuRuntime() = default;

  [[nodiscard]] DeviceType device_type() const noexcept override;
  [[nodiscard]] Result<std::int32_t> device_count() const override;
  [[nodiscard]] Status validate_device(Device device) const override;

  [[nodiscard]] Allocator& allocator(Device device) override;
  [[nodiscard]] Result<Stream> default_stream(Device device) override;
  [[nodiscard]] Result<Stream> create_stream(Device device,
                                             const StreamOptions& options) override;
  [[nodiscard]] Result<std::unique_ptr<Event>> create_event(
      Device device, const EventOptions& options) override;

  [[nodiscard]] Status synchronize(Device device) override;
  [[nodiscard]] RuntimeCapabilities capabilities(Device device) const override;

 private:
  CpuAllocator allocator_;
  std::atomic<std::uint64_t> next_stream_id_{1};
};

[[nodiscard]] Status register_cpu_runtime(RuntimeRegistry& registry);

// The CPU backend owns the production registry assembly. CUDA remains absent
// in CPU-only builds, so querying it returns ErrorCode::kUnavailable.
[[nodiscard]] Status register_builtin_runtimes(RuntimeRegistry& registry);
[[nodiscard]] RuntimeRegistry& default_runtime_registry();

}  // namespace gradyx
