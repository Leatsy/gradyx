#include <benchmark/benchmark.h>

#include "gradyx/core/version.h"

namespace gradyx {
namespace {

void BMVersionAccess(benchmark::State& state) {
  for (auto _ : state) {
    static_cast<void>(_);
    benchmark::DoNotOptimize(version());
  }
}

BENCHMARK(BMVersionAccess);

}  // namespace
}  // namespace gradyx
