add_executable(
  gradyx_benchmarks
  core_types_benchmark.cc
  version_benchmark.cc
)
target_link_libraries(
  gradyx_benchmarks
  PRIVATE gradyx::gradyx benchmark::benchmark_main
)
target_compile_features(gradyx_benchmarks PRIVATE cxx_std_20)
gradyx_set_project_warnings(gradyx_benchmarks)
gradyx_enable_instrumentation(gradyx_benchmarks)
gradyx_enable_clang_tidy(gradyx_benchmarks)

add_test(
  NAME benchmark.smoke
  COMMAND
    gradyx_benchmarks
    --benchmark_min_time=0.01s
    --benchmark_repetitions=1
)
set_tests_properties(benchmark.smoke PROPERTIES LABELS "benchmark-smoke")
