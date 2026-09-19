#include "gradyx/core/data_ptr.h"

#include <gtest/gtest.h>

#include <cstddef>
#include <type_traits>
#include <utility>

namespace gradyx {
namespace {

struct DeleterContext final {
  std::size_t calls{0};
  void* expected_data{nullptr};
};

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
void count_delete(void* data, void* raw_context) noexcept {
  auto* const context = static_cast<DeleterContext*>(raw_context);
  EXPECT_EQ(data, context->expected_data);
  ++context->calls;
}

TEST(DataPtrTest, OwnsAllocationAndRunsDeleterExactlyOnce) {
  int value = 0;
  DeleterContext context{0, &value};
  const Allocation allocation{&value, sizeof(value), Device{DeviceType::kCPU}};

  {
    DataPtr pointer{allocation, &context, count_delete};
    ASSERT_TRUE(pointer);
    EXPECT_EQ(pointer.data(), &value);
    EXPECT_EQ(pointer.bytes(), sizeof(value));
    EXPECT_EQ(pointer.device(), Device{DeviceType::kCPU});
    pointer.reset();
    EXPECT_FALSE(pointer);
  }

  EXPECT_EQ(context.calls, 1U);
}

TEST(DataPtrTest, TransfersOwnershipThroughMoves) {
  int value = 0;
  DeleterContext context{0, &value};
  DataPtr first{
      {&value, sizeof(value), Device{DeviceType::kCPU}}, &context, count_delete};

  DataPtr second{std::move(first)};
  // NOLINTNEXTLINE(bugprone-use-after-move)
  EXPECT_FALSE(first);
  EXPECT_TRUE(second);

  DataPtr third;
  third = std::move(second);
  // NOLINTNEXTLINE(bugprone-use-after-move)
  EXPECT_FALSE(second);
  EXPECT_TRUE(third);
  third.reset();

  EXPECT_EQ(context.calls, 1U);
}

TEST(DataPtrTest, AllowsEmptyZeroByteAllocation) {
  const DataPtr pointer;

  EXPECT_FALSE(pointer);
  EXPECT_EQ(pointer.data(), nullptr);
  EXPECT_EQ(pointer.bytes(), 0U);
  EXPECT_EQ(pointer.device(), Device{DeviceType::kCPU});
}

static_assert(!std::is_copy_constructible_v<DataPtr>);
static_assert(std::is_nothrow_move_constructible_v<DataPtr>);
static_assert(std::is_nothrow_destructible_v<DataPtr>);

}  // namespace
}  // namespace gradyx
