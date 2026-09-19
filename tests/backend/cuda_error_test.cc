#include <gtest/gtest.h>

#include "backend/cuda/error.h"

namespace gradyx::cuda::detail {
namespace {

TEST(CudaErrorContractTest, ConvertsCudaErrorsWithoutLeakingCudaTypes) {
  const Status success =
      cuda_status(cudaSuccess, "cudaSuccess", Device{DeviceType::kCUDA});
  EXPECT_TRUE(success);

  const Status failure =
      cuda_status(cudaErrorInvalidValue, "cudaMalloc", Device{DeviceType::kCUDA, 3});
  EXPECT_FALSE(failure);
  EXPECT_EQ(failure.code(), ErrorCode::kInternal);
  EXPECT_NE(failure.message().find("cudaMalloc"), std::string_view::npos);
  EXPECT_NE(failure.message().find("cuda:3"), std::string_view::npos);
  EXPECT_NE(failure.message().find("cudaErrorInvalidValue"), std::string_view::npos);
}

}  // namespace
}  // namespace gradyx::cuda::detail
