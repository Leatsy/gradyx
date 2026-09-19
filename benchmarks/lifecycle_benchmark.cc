#include <benchmark/benchmark.h>

#include "gradyx/backend/cpu/runtime.h"
#include "gradyx/core/intrusive_ptr.h"
#include "gradyx/runtime/registry.h"

namespace gradyx {
namespace {

class BenchmarkTarget final : public IntrusivePtrTarget {};

void bm_intrusive_ptr_copy(benchmark::State& state) {
  const auto target = make_intrusive<BenchmarkTarget>();
  for (auto _ : state) {
    static_cast<void>(_);
    auto copy = target;
    benchmark::DoNotOptimize(copy.get());
  }
}

void bm_intrusive_ptr_move(benchmark::State& state) {
  for (auto _ : state) {
    static_cast<void>(_);
    auto source = make_intrusive<BenchmarkTarget>();
    auto destination = std::move(source);
    benchmark::DoNotOptimize(destination.get());
  }
}

BENCHMARK(bm_intrusive_ptr_copy);
BENCHMARK(bm_intrusive_ptr_move);

void bm_runtime_registry_lookup(benchmark::State& state) {
  RuntimeRegistry registry;
  if (const Status status = register_cpu_runtime(registry); !status) {
    state.SkipWithError(status.to_string().c_str());
    return;
  }

  for (auto _ : state) {
    static_cast<void>(_);
    auto runtime = registry.get(DeviceType::kCPU);
    benchmark::DoNotOptimize(runtime.value().get());
  }
}

BENCHMARK(bm_runtime_registry_lookup);

}  // namespace
}  // namespace gradyx
