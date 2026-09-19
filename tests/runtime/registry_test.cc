#include "gradyx/runtime/registry.h"

#include <gtest/gtest.h>

#include <atomic>
#include <cstddef>
#include <memory>
#include <thread>
#include <vector>

#include "gradyx/backend/cpu/runtime.h"

namespace gradyx {
namespace {

TEST(RuntimeRegistryTest, ReportsUnavailableAndRejectsInvalidRegistration) {
  RuntimeRegistry registry;

  const auto cuda = registry.get(DeviceType::kCUDA);
  ASSERT_FALSE(cuda);
  EXPECT_EQ(cuda.status().code(), ErrorCode::kUnavailable);

  const auto invalid = registry.get(static_cast<DeviceType>(255));
  ASSERT_FALSE(invalid);
  EXPECT_EQ(invalid.status().code(), ErrorCode::kInvalidArgument);

  EXPECT_EQ(registry.register_runtime(nullptr).code(), ErrorCode::kInvalidArgument);
}

TEST(RuntimeRegistryTest, DefaultRegistryRegistersCpuAndLeavesCudaUnavailable) {
  auto& registry = default_runtime_registry();

  const auto cpu = registry.get(DeviceType::kCPU);
  ASSERT_TRUE(cpu);
  EXPECT_EQ((*cpu)->device_type(), DeviceType::kCPU);

  const auto cuda = registry.get(DeviceType::kCUDA);
  ASSERT_FALSE(cuda);
  EXPECT_EQ(cuda.status().code(), ErrorCode::kUnavailable);
}

TEST(RuntimeRegistryTest, RegistersEachDeviceTypeOnlyOnce) {
  RuntimeRegistry registry;

  ASSERT_TRUE(register_cpu_runtime(registry));
  const auto runtime = registry.get(DeviceType::kCPU);
  ASSERT_TRUE(runtime);
  EXPECT_EQ((*runtime)->device_type(), DeviceType::kCPU);

  const auto duplicate = register_cpu_runtime(registry);
  EXPECT_EQ(duplicate.code(), ErrorCode::kAlreadyExists);
}

TEST(RuntimeRegistryTest, SupportsConcurrentLookups) {
  RuntimeRegistry registry;
  ASSERT_TRUE(register_cpu_runtime(registry));

  constexpr std::size_t thread_count = 8;
  constexpr std::size_t iterations = 10000;
  std::vector<std::thread> threads;
  threads.reserve(thread_count);
  for (std::size_t index = 0; index < thread_count; ++index) {
    threads.emplace_back([&registry] {
      for (std::size_t iteration = 0; iteration < iterations; ++iteration) {
        auto runtime = registry.get(DeviceType::kCPU);
        ASSERT_TRUE(runtime);
        EXPECT_EQ((*runtime)->device_type(), DeviceType::kCPU);
      }
    });
  }
  for (auto& thread : threads) {
    thread.join();
  }
}

TEST(RuntimeRegistryTest, SerializesConcurrentRegistration) {
  RuntimeRegistry registry;

  constexpr std::size_t thread_count = 8;
  std::atomic<std::size_t> successful_registrations{0};
  std::vector<std::thread> threads;
  threads.reserve(thread_count);
  for (std::size_t index = 0; index < thread_count; ++index) {
    threads.emplace_back([&registry, &successful_registrations] {
      if (register_cpu_runtime(registry)) {
        successful_registrations.fetch_add(1, std::memory_order_relaxed);
      }
    });
  }
  for (auto& thread : threads) {
    thread.join();
  }

  EXPECT_EQ(successful_registrations.load(), 1U);
  EXPECT_TRUE(registry.get(DeviceType::kCPU));
}

}  // namespace
}  // namespace gradyx
