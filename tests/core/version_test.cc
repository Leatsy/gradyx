#include "gradyx/core/version.h"

#include <gtest/gtest.h>

#include <string_view>

namespace gradyx {
namespace {

TEST(VersionTest, ReportsProjectVersion) {
  EXPECT_EQ(version(), std::string_view{"0.0.0"});
  EXPECT_EQ(kVersionMajor, 0);
  EXPECT_EQ(kVersionMinor, 0);
  EXPECT_EQ(kVersionPatch, 0);
}

}  // namespace
}  // namespace gradyx
