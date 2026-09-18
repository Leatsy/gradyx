#include "gradyx/core/device.h"

#include <charconv>
#include <limits>
#include <ostream>
#include <system_error>

namespace gradyx {

std::string_view device_type_name(DeviceType type) noexcept {
  switch (type) {
    case DeviceType::kCPU:
      return "cpu";
    case DeviceType::kCUDA:
      return "cuda";
    case DeviceType::kMeta:
      return "meta";
  }
  return "invalid";
}

Result<DeviceType> parse_device_type(std::string_view text) {
  if (text == "cpu") {
    return DeviceType::kCPU;
  }
  if (text == "cuda") {
    return DeviceType::kCUDA;
  }
  if (text == "meta") {
    return DeviceType::kMeta;
  }
  return Status::invalid_argument("unknown device type: " + std::string{text});
}

Result<Device> Device::parse(std::string_view text) {
  if (text.empty()) {
    return Status::invalid_argument("device string must not be empty");
  }

  const auto separator = text.find(':');
  const auto type_text = text.substr(0, separator);
  auto parsed_type = parse_device_type(type_text);
  if (!parsed_type) {
    return parsed_type.status();
  }

  DeviceIndex index = kDefaultDeviceIndex;
  if (separator != std::string_view::npos) {
    const auto index_text = text.substr(separator + 1);
    if (index_text.empty()) {
      return Status::invalid_argument("device index must not be empty");
    }

    std::int64_t wide_index = 0;
    const auto* first = index_text.data();
    const auto* last = first + index_text.size();
    const auto [end, error] = std::from_chars(first, last, wide_index);
    if (error != std::errc{} || end != last) {
      return Status::invalid_argument("invalid device index: " +
                                      std::string{index_text});
    }
    if (wide_index < 0 || wide_index > std::numeric_limits<DeviceIndex>::max()) {
      return Status::out_of_range("device index is out of range: " +
                                  std::string{index_text});
    }
    index = static_cast<DeviceIndex>(wide_index);
  }

  return Device{*parsed_type, index};
}

std::string Device::str() const {
  return std::string{device_type_name(type_)} + ':' + std::to_string(index_);
}

std::ostream& operator<<(std::ostream& output, Device device) {
  return output << device.str();
}

}  // namespace gradyx
