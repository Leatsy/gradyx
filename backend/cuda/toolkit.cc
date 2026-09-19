#if defined(GRADYX_HAS_CUDA)

#include <cstdint>

#include "backend/cuda/error.h"

namespace gradyx::cuda::detail {

// Keep CUDA headers and toolkit ABI dependencies inside the CUDA-only target.
[[nodiscard]] std::int32_t toolkit_runtime_version() noexcept { return CUDART_VERSION; }

[[nodiscard]] Status verify_toolkit_boundary() {
  return cuda_status(cudaSuccess, "cudaSuccess", Device{DeviceType::kCUDA});
}

}  // namespace gradyx::cuda::detail

#endif
