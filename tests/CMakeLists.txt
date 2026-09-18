include(GoogleTest)

add_executable(
  gradyx_core_tests
  core/checked_math_test.cc
  core/device_test.cc
  core/dtype_test.cc
  core/scalar_test.cc
  core/shape_test.cc
  core/status_result_test.cc
  core/version_test.cc
)
target_link_libraries(gradyx_core_tests PRIVATE gradyx::gradyx GTest::gtest_main)
target_compile_features(gradyx_core_tests PRIVATE cxx_std_20)
gradyx_set_project_warnings(gradyx_core_tests)
gradyx_enable_instrumentation(gradyx_core_tests)
gradyx_enable_clang_tidy(gradyx_core_tests)

gtest_discover_tests(
  gradyx_core_tests
  TEST_PREFIX "unit."
  DISCOVERY_MODE PRE_TEST
  PROPERTIES LABELS "unit"
)
