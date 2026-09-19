#pragma once

#include <cstdint>

#include "gradyx/core/device.h"

namespace gradyx {

struct StreamOptions final {
  bool non_blocking{false};
};

class Stream final {
 public:
  // Device is initialized with a known-valid CPU value; clang-tidy cannot
  // prove that the Device validation path is non-throwing here.
  // NOLINTNEXTLINE(bugprone-exception-escape)
  constexpr Stream() noexcept = default;

  [[nodiscard]] constexpr Device device() const noexcept { return device_; }
  [[nodiscard]] constexpr std::uint64_t id() const noexcept { return id_; }
  [[nodiscard]] constexpr bool is_default() const noexcept { return is_default_; }
  [[nodiscard]] constexpr explicit operator bool() const noexcept { return valid_; }

  friend constexpr bool operator==(Stream, Stream) = default;

 private:
  friend class Runtime;
  friend class CpuRuntime;

  constexpr Stream(Device device, std::uint64_t id, bool is_default) noexcept
      : device_(device), id_(id), is_default_(is_default), valid_(true) {}

  Device device_{DeviceType::kCPU};
  std::uint64_t id_{0};
  bool is_default_{false};
  bool valid_{false};
};

}  // namespace gradyx
