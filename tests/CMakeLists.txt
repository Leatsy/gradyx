include(GoogleTest)
find_package(Threads REQUIRED)

set(gradyx_test_label_suffix)
if(GRADYX_ENABLE_ASAN OR GRADYX_ENABLE_UBSAN OR GRADYX_ENABLE_TSAN
   OR GRADYX_ENABLE_LSAN)
  set(gradyx_test_label_suffix sanitizer)
endif()

if(gradyx_test_label_suffix)
  set(gradyx_unit_test_label "${gradyx_test_label_suffix}")
  set(gradyx_contract_test_label "${gradyx_test_label_suffix}")
  set(gradyx_integration_test_label "${gradyx_test_label_suffix}")
else()
  set(gradyx_unit_test_label unit)
  set(gradyx_contract_test_label contract)
  set(gradyx_integration_test_label integration)
endif()

add_executable(
  gradyx_core_tests
  core/checked_math_test.cc
  core/data_ptr_test.cc
  core/device_test.cc
  core/dtype_test.cc
  core/intrusive_ptr_test.cc
  core/scalar_test.cc
  core/shape_test.cc
  core/status_result_test.cc
  core/version_test.cc
)
target_link_libraries(
  gradyx_core_tests
  PRIVATE gradyx::gradyx GTest::gtest_main Threads::Threads
)
target_compile_features(gradyx_core_tests PRIVATE cxx_std_20)
gradyx_set_project_warnings(gradyx_core_tests)
gradyx_enable_instrumentation(gradyx_core_tests)
gradyx_enable_clang_tidy(gradyx_core_tests)

gtest_discover_tests(
  gradyx_core_tests
  TEST_PREFIX "unit."
  DISCOVERY_MODE PRE_TEST
  PROPERTIES LABELS "${gradyx_unit_test_label}"
)

add_executable(gradyx_runtime_contract_tests runtime/runtime_contract_test.cc)
target_link_libraries(
  gradyx_runtime_contract_tests
  PRIVATE gradyx::gradyx GTest::gtest_main Threads::Threads
)
target_compile_features(gradyx_runtime_contract_tests PRIVATE cxx_std_20)
gradyx_set_project_warnings(gradyx_runtime_contract_tests)
gradyx_enable_instrumentation(gradyx_runtime_contract_tests)
gradyx_enable_clang_tidy(gradyx_runtime_contract_tests)

gtest_discover_tests(
  gradyx_runtime_contract_tests
  TEST_PREFIX "contract."
  DISCOVERY_MODE PRE_TEST
  PROPERTIES LABELS "${gradyx_contract_test_label}"
)

add_executable(gradyx_runtime_registry_tests runtime/registry_test.cc)
target_link_libraries(
  gradyx_runtime_registry_tests
  PRIVATE gradyx::gradyx GTest::gtest_main Threads::Threads
)
target_compile_features(gradyx_runtime_registry_tests PRIVATE cxx_std_20)
gradyx_set_project_warnings(gradyx_runtime_registry_tests)
gradyx_enable_instrumentation(gradyx_runtime_registry_tests)
gradyx_enable_clang_tidy(gradyx_runtime_registry_tests)

gtest_discover_tests(
  gradyx_runtime_registry_tests
  TEST_PREFIX "integration."
  DISCOVERY_MODE PRE_TEST
  PROPERTIES LABELS "${gradyx_integration_test_label}"
)

if(GRADYX_ENABLE_CUDA)
  add_executable(gradyx_cuda_contract_tests backend/cuda_error_test.cc)
  target_include_directories(
    gradyx_cuda_contract_tests
    PRIVATE "${PROJECT_SOURCE_DIR}"
  )
  target_compile_definitions(gradyx_cuda_contract_tests PRIVATE GRADYX_HAS_CUDA=1)
  target_link_libraries(
    gradyx_cuda_contract_tests
    PRIVATE gradyx::backend_cuda CUDA::cudart GTest::gtest_main
  )
  target_compile_features(gradyx_cuda_contract_tests PRIVATE cxx_std_20)
  gradyx_set_project_warnings(gradyx_cuda_contract_tests)
  gradyx_enable_instrumentation(gradyx_cuda_contract_tests)
  gradyx_enable_clang_tidy(gradyx_cuda_contract_tests)

  gtest_discover_tests(
    gradyx_cuda_contract_tests
    TEST_PREFIX "contract."
    DISCOVERY_MODE PRE_TEST
    PROPERTIES LABELS "cuda"
  )
endif()
