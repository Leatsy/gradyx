#pragma once

#include <cstdint>
#include <functional>
#include <iosfwd>
#include <stdexcept>
#include <string>
#include <string_view>

#include "gradyx/core/result.h"

namespace gradyx {

using DeviceIndex = std::int32_t;

inline constexpr DeviceIndex kDefaultDeviceIndex = 0;

enum class DeviceType : std::uint8_t {
  kCPU = 0,
  kCUDA = 1,
  kMeta = 2,
};

[[nodiscard]] constexpr bool is_valid_device_type(DeviceType type) noexcept {
  return type == DeviceType::kCPU || type == DeviceType::kCUDA ||
         type == DeviceType::kMeta;
}

[[nodiscard]] std::string_view device_type_name(DeviceType type) noexcept;
[[nodiscard]] Result<DeviceType> parse_device_type(std::string_view text);

class Device final {
 public:
  constexpr explicit Device(DeviceType type, DeviceIndex index = kDefaultDeviceIndex)
      : type_(type), index_(index) {
    if (!is_valid_device_type(type)) {
      throw std::invalid_argument("invalid device type");
    }
    if (index < 0) {
      throw std::invalid_argument("device index must be non-negative");
    }
  }

  [[nodiscard]] static Result<Device> parse(std::string_view text);

  [[nodiscard]] constexpr DeviceType type() const noexcept { return type_; }
  [[nodiscard]] constexpr DeviceIndex index() const noexcept { return index_; }
  [[nodiscard]] std::string str() const;

  friend constexpr bool operator==(Device, Device) = default;

 private:
  DeviceType type_;
  DeviceIndex index_;
};

std::ostream& operator<<(std::ostream& output, Device device);

}  // namespace gradyx

template <>
struct std::hash<gradyx::Device> {
  [[nodiscard]] std::size_t operator()(gradyx::Device device) const noexcept {
    const auto type = static_cast<std::size_t>(device.type());
    const auto index = static_cast<std::size_t>(device.index());
    return (type * static_cast<std::size_t>(0x9e3779b9U)) ^ index;
  }
};
