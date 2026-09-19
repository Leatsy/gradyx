#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>

#include "gradyx/backend/cpu/runtime.h"

namespace gradyx {
namespace {

TEST(CpuRuntimeContractTest, ValidatesExactlyCpuZero) {
  CpuRuntime runtime;

  EXPECT_TRUE(runtime.validate_device(Device{DeviceType::kCPU}));
  EXPECT_FALSE(runtime.validate_device(Device{DeviceType::kCPU, 1}));
  EXPECT_FALSE(runtime.validate_device(Device{DeviceType::kCUDA}));
  const auto count = runtime.device_count();
  ASSERT_TRUE(count);
  EXPECT_EQ(*count, 1);
}

TEST(CpuRuntimeContractTest, AllocatesAlignedMemoryAndReleasesIt) {
  CpuRuntime runtime;
  auto allocation = runtime.allocator(Device{DeviceType::kCPU})
                        .allocate(256, 64, Device{DeviceType::kCPU});
  ASSERT_TRUE(allocation) << allocation.status().to_string();

  EXPECT_EQ(allocation->bytes(), 256U);
  EXPECT_EQ(allocation->device(), Device{DeviceType::kCPU});
  EXPECT_EQ(reinterpret_cast<std::uintptr_t>(allocation->data()) % 64U, 0U);
  std::memset(allocation->data(), 0x5a, allocation->bytes());
}

TEST(CpuRuntimeContractTest, HandlesEmptyAndInvalidAllocations) {
  CpuRuntime runtime;
  auto& allocator = runtime.allocator(Device{DeviceType::kCPU});

  const auto empty =
      allocator.allocate(0, alignof(std::max_align_t), Device{DeviceType::kCPU});
  ASSERT_TRUE(empty);
  EXPECT_FALSE(*empty);

  const auto invalid = allocator.allocate(64, 3, Device{DeviceType::kCPU});
  ASSERT_FALSE(invalid);
  EXPECT_EQ(invalid.status().code(), ErrorCode::kInvalidArgument);

  const auto wrong_device = allocator.allocate(64, 64, Device{DeviceType::kCUDA});
  ASSERT_FALSE(wrong_device);
  EXPECT_EQ(wrong_device.status().code(), ErrorCode::kInvalidArgument);

  const auto exhausted = allocator.allocate(std::numeric_limits<std::size_t>::max(), 64,
                                            Device{DeviceType::kCPU});
  ASSERT_FALSE(exhausted);
  EXPECT_EQ(exhausted.status().code(), ErrorCode::kResourceExhausted);
  EXPECT_NE(exhausted.status().message().find("CPU allocator"), std::string_view::npos);
  EXPECT_NE(exhausted.status().message().find("cpu:0"), std::string_view::npos);
}

TEST(CpuRuntimeContractTest, ProvidesSynchronousStreamsAndEvents) {
  CpuRuntime runtime;
  const Device cpu{DeviceType::kCPU};
  const auto default_stream = runtime.default_stream(cpu);
  ASSERT_TRUE(default_stream);
  EXPECT_TRUE(default_stream->is_default());
  const auto repeated_default_stream = runtime.default_stream(cpu);
  ASSERT_TRUE(repeated_default_stream);
  EXPECT_EQ(*default_stream, *repeated_default_stream);

  const auto stream = runtime.create_stream(cpu, {});
  ASSERT_TRUE(stream);
  EXPECT_FALSE(stream->is_default());
  EXPECT_NE(stream->id(), default_stream->id());

  auto event = runtime.create_event(cpu, {});
  ASSERT_TRUE(event);
  EXPECT_FALSE((*event)->query());
  EXPECT_EQ((*event)->query().status().code(), ErrorCode::kFailedPrecondition);
  EXPECT_EQ((*event)->wait(Stream{}).code(), ErrorCode::kInvalidArgument);

  ASSERT_TRUE((*event)->record(*stream));
  EXPECT_TRUE(*(*event)->query());
  EXPECT_TRUE((*event)->synchronize());
  EXPECT_TRUE((*event)->wait(*default_stream));
  EXPECT_TRUE((*event)->record(*default_stream));

  EXPECT_TRUE(runtime.synchronize(cpu));
  EXPECT_FALSE(runtime.create_stream(Device{DeviceType::kCUDA}, {}));
}

TEST(CpuRuntimeContractTest, ReportsCpuCapabilities) {
  CpuRuntime runtime;
  const auto capabilities = runtime.capabilities(Device{DeviceType::kCPU});

  EXPECT_FALSE(capabilities.asynchronous_execution);
  EXPECT_FALSE(capabilities.pinned_memory);
  EXPECT_GE(capabilities.minimum_alignment, alignof(std::max_align_t));
}

}  // namespace
}  // namespace gradyx
