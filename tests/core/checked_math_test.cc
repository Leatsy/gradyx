#include "gradyx/core/checked_math.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <limits>

namespace gradyx {
namespace {

TEST(CheckedAddTest, AddsSignedAndUnsignedValues) {
  const auto signed_result = checked_add<std::int64_t>(-10, 4);
  const auto unsigned_result = checked_add<std::uint64_t>(10, 4);

  ASSERT_TRUE(signed_result);
  ASSERT_TRUE(unsigned_result);
  EXPECT_EQ(*signed_result, -6);
  EXPECT_EQ(*unsigned_result, 14U);
}

TEST(CheckedAddTest, DetectsBothSignedOverflowDirections) {
  const auto positive =
      checked_add(std::numeric_limits<std::int64_t>::max(), std::int64_t{1});
  const auto negative =
      checked_add(std::numeric_limits<std::int64_t>::min(), std::int64_t{-1});

  ASSERT_FALSE(positive);
  ASSERT_FALSE(negative);
  EXPECT_EQ(positive.status().code(), ErrorCode::kOverflow);
  EXPECT_EQ(negative.status().code(), ErrorCode::kOverflow);
}

TEST(CheckedMultiplyTest, HandlesSignsAndZero) {
  const auto negative = checked_multiply<std::int64_t>(-7, 6);
  const auto zero =
      checked_multiply<std::int64_t>(std::numeric_limits<std::int64_t>::min(), 0);

  ASSERT_TRUE(negative);
  ASSERT_TRUE(zero);
  EXPECT_EQ(*negative, -42);
  EXPECT_EQ(*zero, 0);
}

TEST(CheckedMultiplyTest, DetectsSignedAndUnsignedOverflow) {
  const auto signed_result =
      checked_multiply(std::numeric_limits<std::int64_t>::min(), std::int64_t{-1});
  const auto unsigned_result =
      checked_multiply(std::numeric_limits<std::uint64_t>::max(), std::uint64_t{2});

  ASSERT_FALSE(signed_result);
  ASSERT_FALSE(unsigned_result);
  EXPECT_EQ(signed_result.status().code(), ErrorCode::kOverflow);
  EXPECT_EQ(unsigned_result.status().code(), ErrorCode::kOverflow);
}

}  // namespace
}  // namespace gradyx
