#include "gradyx/core/device.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_set>

namespace gradyx {
namespace {

static_assert(std::is_trivially_copyable_v<Device>);

TEST(DeviceTypeTest, ParsesKnownTypes) {
  EXPECT_EQ(*parse_device_type("cpu"), DeviceType::kCPU);
  EXPECT_EQ(*parse_device_type("cuda"), DeviceType::kCUDA);
  EXPECT_EQ(*parse_device_type("meta"), DeviceType::kMeta);
}

TEST(DeviceTypeTest, RejectsUnknownTypeAndHandlesInvalidEnum) {
  const auto unknown = parse_device_type("tpu");
  ASSERT_FALSE(unknown);
  EXPECT_EQ(unknown.status().code(), ErrorCode::kInvalidArgument);

  const auto invalid = static_cast<DeviceType>(255);
  EXPECT_EQ(device_type_name(invalid), "invalid");
}

TEST(DeviceTest, UsesCanonicalDefaultIndex) {
  const Device cpu{DeviceType::kCPU};
  const Device cuda{DeviceType::kCUDA, 3};

  EXPECT_EQ(cpu.str(), "cpu:0");
  EXPECT_EQ(cuda.str(), "cuda:3");
  EXPECT_EQ(cpu.index(), kDefaultDeviceIndex);
}

TEST(DeviceTest, ParsesAndFormatsRoundTrip) {
  for (const std::string text : {"cpu", "cpu:7", "cuda", "cuda:2", "meta"}) {
    const auto parsed = Device::parse(text);
    ASSERT_TRUE(parsed) << parsed.status().to_string();

    const auto reparsed = Device::parse(parsed->str());
    ASSERT_TRUE(reparsed);
    EXPECT_EQ(*parsed, *reparsed);
  }

  std::ostringstream output;
  output << Device{DeviceType::kCUDA, 4};
  EXPECT_EQ(output.str(), "cuda:4");
}

TEST(DeviceTest, RejectsMalformedInput) {
  for (const std::string text :
       {"", "tpu", "cpu:", "cpu:-1", "cpu:1:2", "cpu:+1", "cpu:9999999999"}) {
    EXPECT_FALSE(Device::parse(text)) << text;
  }
}

TEST(DeviceTest, ConstructorRejectsInvalidState) {
  EXPECT_THROW(static_cast<void>(Device{DeviceType::kCPU, -1}), std::invalid_argument);
  EXPECT_THROW(static_cast<void>(Device{static_cast<DeviceType>(255), 0}),
               std::invalid_argument);
}

TEST(DeviceTest, SupportsHashContainers) {
  std::unordered_set<Device> devices;
  devices.emplace(DeviceType::kCPU, 0);
  devices.emplace(DeviceType::kCUDA, 0);
  devices.emplace(DeviceType::kCUDA, 0);

  EXPECT_EQ(devices.size(), 2U);
}

}  // namespace
}  // namespace gradyx
