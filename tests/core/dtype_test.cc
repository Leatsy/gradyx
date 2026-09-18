#include "gradyx/core/dtype.h"

#include <gtest/gtest.h>

#include <array>
#include <complex>
#include <cstdint>
#include <sstream>
#include <string_view>
#include <type_traits>

namespace gradyx {
namespace {

static_assert(sizeof(Float16) == 2);
static_assert(sizeof(BFloat16) == 2);
static_assert(std::is_trivially_copyable_v<Float16>);
static_assert(std::is_trivially_copyable_v<BFloat16>);
static_assert(dtype_of<bool>() == DType::kBool);
static_assert(dtype_of<std::uint8_t>() == DType::kUInt8);
static_assert(dtype_of<std::int64_t>() == DType::kInt64);
static_assert(dtype_of<std::uint64_t>() == DType::kUInt64);
static_assert(dtype_of<float>() == DType::kFloat32);
static_assert(dtype_of<std::complex<double>>() == DType::kComplex128);
static_assert(!kHasDTypeV<char>);

TEST(LowPrecisionTypeTest, PreservesRawBits) {
  constexpr auto float16 = Float16::from_bits(0x3c00U);
  constexpr auto bfloat16 = BFloat16::from_bits(0x3f80U);

  static_assert(float16.bits() == 0x3c00U);
  static_assert(bfloat16.bits() == 0x3f80U);
  EXPECT_EQ(float16.bits(), 0x3c00U);
  EXPECT_EQ(bfloat16.bits(), 0x3f80U);
}

TEST(DTypeTest, ProvidesMetadataAndRoundTripParsing) {
  constexpr std::array dtypes{
      DType::kBool,    DType::kUInt8,   DType::kInt8,      DType::kInt16,
      DType::kInt32,   DType::kInt64,   DType::kFloat16,   DType::kBFloat16,
      DType::kFloat32, DType::kFloat64, DType::kComplex64, DType::kComplex128,
      DType::kUInt64,
  };

  for (const DType dtype : dtypes) {
    const auto info = dtype_info(dtype);
    ASSERT_TRUE(info);
    EXPECT_FALSE(info->name.empty());
    EXPECT_GT(info->size_bytes, 0U);
    EXPECT_GT(info->alignment, 0U);

    const auto parsed = parse_dtype(info->name);
    ASSERT_TRUE(parsed);
    EXPECT_EQ(*parsed, dtype);
    EXPECT_EQ(*element_size(dtype), info->size_bytes);
  }
}

TEST(DTypeTest, ClassifiesTypes) {
  EXPECT_TRUE(is_boolean(DType::kBool));
  EXPECT_TRUE(is_integral(DType::kInt32));
  EXPECT_TRUE(is_integral(DType::kUInt64));
  EXPECT_TRUE(is_floating(DType::kBFloat16));
  EXPECT_TRUE(is_complex(DType::kComplex128));

  EXPECT_FALSE(is_integral(DType::kBool));
  EXPECT_FALSE(is_floating(DType::kInt64));
  EXPECT_FALSE(is_complex(DType::kFloat64));
}

TEST(DTypeTest, HandlesInvalidValuesWithoutTableAccess) {
  const auto invalid = static_cast<DType>(255);

  EXPECT_FALSE(dtype_info(invalid));
  EXPECT_FALSE(dtype_name(invalid));
  EXPECT_FALSE(element_size(invalid));
  EXPECT_FALSE(is_boolean(invalid));
  EXPECT_FALSE(is_integral(invalid));
  EXPECT_FALSE(is_floating(invalid));
  EXPECT_FALSE(is_complex(invalid));

  std::ostringstream output;
  output << invalid;
  EXPECT_EQ(output.str(), "invalid_dtype(255)");
}

TEST(DTypeTest, RejectsUnknownNames) {
  const auto result = parse_dtype("float128");

  ASSERT_FALSE(result);
  EXPECT_EQ(result.status().code(), ErrorCode::kInvalidArgument);
}

}  // namespace
}  // namespace gradyx
