#include "gradyx/core/scalar.h"

#include <gtest/gtest.h>

#include <cmath>
#include <complex>
#include <cstdint>
#include <limits>

namespace gradyx {
namespace {

TEST(ScalarTest, ReportsCanonicalStorageDType) {
  const Scalar complex_scalar{std::complex<float>{1.0F, 2.0F}};

  EXPECT_EQ(Scalar{true}.dtype(), DType::kBool);
  EXPECT_EQ(Scalar{std::int8_t{-3}}.dtype(), DType::kInt64);
  EXPECT_EQ(Scalar{std::uint64_t{7}}.dtype(), DType::kUInt64);
  EXPECT_EQ(Scalar{1.5F}.dtype(), DType::kFloat64);
  EXPECT_EQ(complex_scalar.dtype(), DType::kComplex128);
}

TEST(ScalarTest, ConvertsBetweenCompatibleTypes) {
  const auto integer = Scalar{42.0}.to<std::int32_t>();
  const auto floating = Scalar{std::int64_t{-7}}.to<float>();
  const auto complex = Scalar{std::uint64_t{9}}.to<std::complex<double>>();

  ASSERT_TRUE(integer);
  ASSERT_TRUE(floating);
  ASSERT_TRUE(complex);
  EXPECT_EQ(*integer, 42);
  EXPECT_FLOAT_EQ(*floating, -7.0F);
  EXPECT_EQ(*complex, std::complex<double>(9.0, 0.0));
}

TEST(ScalarTest, RejectsLossyOrOutOfRangeIntegerConversions) {
  const auto fractional = Scalar{1.5}.to<std::int64_t>();
  const auto non_finite =
      Scalar{std::numeric_limits<double>::infinity()}.to<std::int64_t>();
  const auto negative = Scalar{-1}.to<std::uint64_t>();
  const auto overflowing =
      Scalar{std::numeric_limits<std::uint64_t>::max()}.to<std::int64_t>();

  EXPECT_EQ(fractional.status().code(), ErrorCode::kInvalidArgument);
  EXPECT_EQ(non_finite.status().code(), ErrorCode::kOutOfRange);
  EXPECT_EQ(negative.status().code(), ErrorCode::kOutOfRange);
  EXPECT_EQ(overflowing.status().code(), ErrorCode::kOutOfRange);
}

TEST(ScalarTest, RequiresExactBooleanValues) {
  EXPECT_EQ(*Scalar{0}.to<bool>(), false);
  EXPECT_EQ(*Scalar{1.0}.to<bool>(), true);

  const auto invalid_integer = Scalar{2}.to<bool>();
  const auto invalid_complex = Scalar{std::complex<double>{1.0, 1.0}}.to<bool>();
  EXPECT_EQ(invalid_integer.status().code(), ErrorCode::kInvalidArgument);
  EXPECT_EQ(invalid_complex.status().code(), ErrorCode::kInvalidArgument);
}

TEST(ScalarTest, RejectsNonRealComplexConversions) {
  const Scalar scalar{std::complex<double>{3.0, 2.0}};

  const auto real = scalar.to<double>();
  const auto integer = scalar.to<std::int64_t>();
  const auto complex = scalar.to<std::complex<float>>();

  EXPECT_EQ(real.status().code(), ErrorCode::kInvalidArgument);
  EXPECT_EQ(integer.status().code(), ErrorCode::kInvalidArgument);
  ASSERT_TRUE(complex);
  EXPECT_EQ(*complex, std::complex<float>(3.0F, 2.0F));
}

TEST(ScalarTest, ChecksFloatingPointNarrowingRange) {
  const auto result = Scalar{std::numeric_limits<double>::max()}.to<float>();

  ASSERT_FALSE(result);
  EXPECT_EQ(result.status().code(), ErrorCode::kOutOfRange);
}

TEST(ScalarTest, PreservesFloatingPointSpecialValues) {
  const auto infinity = Scalar{std::numeric_limits<double>::infinity()}.to<float>();
  const auto nan = Scalar{std::numeric_limits<double>::quiet_NaN()}.to<float>();

  ASSERT_TRUE(infinity);
  ASSERT_TRUE(nan);
  EXPECT_TRUE(std::isinf(*infinity));
  EXPECT_TRUE(std::isnan(*nan));
}

}  // namespace
}  // namespace gradyx
