#pragma once

#if defined(GRADYX_HAS_CUDA)

#include <cuda_runtime_api.h>

#include <source_location>
#include <string>
#include <string_view>

#include "gradyx/core/device.h"
#include "gradyx/core/status.h"

namespace gradyx::cuda::detail {

[[nodiscard]] inline Status cuda_status(
    cudaError_t error, std::string_view api, Device device,
    std::source_location location = std::source_location::current()) {
  if (error == cudaSuccess) {
    return {};
  }
  return Status::error(ErrorCode::kInternal,
                       "CUDA API " + std::string(api) + " failed on " + device.str() +
                           " (" + std::string(cudaGetErrorName(error)) +
                           "): " + cudaGetErrorString(error),
                       location);
}

}  // namespace gradyx::cuda::detail

#endif
