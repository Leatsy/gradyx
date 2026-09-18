#include <benchmark/benchmark.h>

#include "gradyx/core/device.h"
#include "gradyx/core/dtype.h"
#include "gradyx/core/shape.h"

namespace gradyx {
namespace {

void bm_device_parse(benchmark::State& state) {
  for (auto _ : state) {
    static_cast<void>(_);
    auto device = Device::parse("cuda:7");
    benchmark::DoNotOptimize(device.value().index());
  }
}

void bm_dtype_info(benchmark::State& state) {
  for (auto _ : state) {
    static_cast<void>(_);
    auto info = dtype_info(DType::kFloat32);
    benchmark::DoNotOptimize(info.value().size_bytes);
  }
}

void bm_shape_numel(benchmark::State& state) {
  const Shape shape{32, 128, 768};
  for (auto _ : state) {
    static_cast<void>(_);
    auto count = shape.numel();
    benchmark::DoNotOptimize(count.value());
  }
}

BENCHMARK(bm_device_parse);
BENCHMARK(bm_dtype_info);
BENCHMARK(bm_shape_numel);

}  // namespace
}  // namespace gradyx
