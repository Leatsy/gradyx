#include "gradyx/core/shape.h"

#include <gtest/gtest.h>

#include <array>
#include <cstdint>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <vector>

#include "gradyx/core/layout.h"

namespace gradyx {
namespace {

TEST(ShapeTest, RepresentsScalarAndZeroSizedShapes) {
  const Shape scalar;
  const Shape zero_sized{2, 0, 3};

  EXPECT_EQ(scalar.rank(), 0U);
  EXPECT_TRUE(scalar.empty());
  ASSERT_TRUE(scalar.numel());
  EXPECT_EQ(*scalar.numel(), 1U);

  EXPECT_EQ(zero_sized.rank(), 3U);
  ASSERT_TRUE(zero_sized.numel());
  EXPECT_EQ(*zero_sized.numel(), 0U);
}

TEST(ShapeTest, RejectsNegativeDimensions) {
  EXPECT_THROW(static_cast<void>(Shape{2, -1, 3}), std::invalid_argument);

  const std::array<Dim, 3> dims{2, -1, 3};
  const auto shape = Shape::from_dims(dims);
  ASSERT_FALSE(shape);
  EXPECT_EQ(shape.status().code(), ErrorCode::kInvalidArgument);
}

TEST(ShapeTest, ChecksAxisAndElementCountOverflow) {
  const Shape shape{2, 3, 4};
  EXPECT_EQ(shape[1], 3);
  EXPECT_THROW(static_cast<void>(shape[3]), std::out_of_range);

  const Shape overflowing{std::numeric_limits<Dim>::max(), 3};
  const auto count = overflowing.numel();
  ASSERT_FALSE(count);
  EXPECT_EQ(count.status().code(), ErrorCode::kOverflow);
}

TEST(StridesTest, ComputesCanonicalContiguousStrides) {
  const auto scalar_strides = contiguous_strides(Shape{});
  const auto dense_strides = contiguous_strides(Shape{2, 3, 4});
  const auto zero_strides = contiguous_strides(Shape{2, 0, 3});

  ASSERT_TRUE(scalar_strides);
  ASSERT_TRUE(dense_strides);
  ASSERT_TRUE(zero_strides);
  EXPECT_TRUE(scalar_strides->empty());
  EXPECT_EQ(*dense_strides, (Strides{12, 4, 1}));
  EXPECT_EQ(*zero_strides, (Strides{3, 3, 1}));
}

TEST(StridesTest, ValidatesRankAndContiguity) {
  const Shape shape{2, 3};
  const std::array<Stride, 2> contiguous{3, 1};
  const std::array<Stride, 2> transposed{1, 2};
  const std::array<Stride, 1> wrong_rank{1};
  const std::array<Stride, 2> negative{3, -1};

  EXPECT_TRUE(*is_contiguous(shape, contiguous));
  EXPECT_FALSE(*is_contiguous(shape, transposed));
  EXPECT_FALSE(*is_contiguous(shape, negative));

  const auto invalid = is_contiguous(shape, wrong_rank);
  ASSERT_FALSE(invalid);
  EXPECT_EQ(invalid.status().code(), ErrorCode::kInvalidArgument);
}

TEST(StridesTest, DetectsStrideOverflow) {
  const Shape shape{2, std::numeric_limits<Dim>::max(), 2};
  const auto strides = contiguous_strides(shape);

  ASSERT_FALSE(strides);
  EXPECT_EQ(strides.status().code(), ErrorCode::kOverflow);
}

TEST(LayoutTest, ParsesAndFormatsLayout) {
  const auto layout = parse_layout("strided");
  ASSERT_TRUE(layout);
  EXPECT_EQ(*layout, Layout::kStrided);
  EXPECT_EQ(layout_name(*layout), "strided");

  std::ostringstream output;
  output << *layout;
  EXPECT_EQ(output.str(), "strided");
}

TEST(LayoutTest, RejectsUnknownAndInvalidLayouts) {
  EXPECT_FALSE(parse_layout("sparse"));

  const auto invalid = static_cast<Layout>(255);
  EXPECT_FALSE(is_valid_layout(invalid));
  EXPECT_EQ(layout_name(invalid), "invalid");
}

}  // namespace
}  // namespace gradyx
