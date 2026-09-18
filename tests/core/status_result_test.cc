#include <gtest/gtest.h>

#include <memory>
#include <string>

#include "gradyx/core/result.h"
#include "gradyx/core/status.h"

namespace gradyx {
namespace {

TEST(StatusTest, DefaultStatusIsOk) {
  const Status status;

  EXPECT_TRUE(status.ok());
  EXPECT_TRUE(static_cast<bool>(status));
  EXPECT_EQ(status.code(), ErrorCode::kOk);
  EXPECT_EQ(status.to_string(), "Ok");
}

TEST(StatusTest, ErrorContainsCodeMessageAndLocation) {
  const auto status = Status::invalid_argument("bad input");

  EXPECT_FALSE(status.ok());
  EXPECT_EQ(status.code(), ErrorCode::kInvalidArgument);
  EXPECT_EQ(status.message(), "bad input");
  EXPECT_NE(status.location().line(), 0U);
  EXPECT_NE(status.to_string().find("InvalidArgument: bad input"), std::string::npos);
}

TEST(StatusTest, RejectsOkCodeForErrorFactory) {
  const auto status = Status::error(ErrorCode::kOk, "invalid construction");

  EXPECT_EQ(status.code(), ErrorCode::kInternal);
}

TEST(ResultTest, StoresValuesAndErrors) {
  Result<int> value{42};
  Result<int> error{Status::out_of_range("outside range")};

  ASSERT_TRUE(value);
  EXPECT_EQ(*value, 42);
  EXPECT_TRUE(value.status().ok());

  ASSERT_FALSE(error);
  EXPECT_EQ(error.status().code(), ErrorCode::kOutOfRange);
  EXPECT_THROW(static_cast<void>(error.value()), std::logic_error);
}

TEST(ResultTest, SupportsMoveOnlyValues) {
  Result<std::unique_ptr<int>> result{std::make_unique<int>(7)};

  ASSERT_TRUE(result);
  auto value = std::move(result).value();
  ASSERT_NE(value, nullptr);
  EXPECT_EQ(*value, 7);
}

}  // namespace
}  // namespace gradyx
