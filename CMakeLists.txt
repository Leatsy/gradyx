cmake_minimum_required(VERSION 3.24)

project(
  GradyX
  VERSION 0.0.0
  DESCRIPTION "A CPU and CUDA deep learning framework"
  LANGUAGES CXX
)

include(CTest)
include(GNUInstallDirs)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

option(GRADYX_BUILD_TESTS "Build GradyX tests" ${BUILD_TESTING})
option(GRADYX_BUILD_BENCHMARKS "Build GradyX benchmarks" OFF)
option(GRADYX_ENABLE_CUDA "Build the CUDA backend" OFF)
option(GRADYX_ENABLE_ASAN "Enable AddressSanitizer" OFF)
option(GRADYX_ENABLE_UBSAN "Enable UndefinedBehaviorSanitizer" OFF)
option(GRADYX_ENABLE_TSAN "Enable ThreadSanitizer" OFF)
option(GRADYX_ENABLE_LSAN "Enable LeakSanitizer" OFF)
option(GRADYX_ENABLE_COVERAGE "Enable coverage instrumentation" OFF)
option(GRADYX_ENABLE_CLANG_TIDY "Run clang-tidy during compilation" OFF)
option(GRADYX_WARNINGS_AS_ERRORS "Treat project warnings as errors" OFF)
option(
  GRADYX_USE_SYSTEM_DEPENDENCIES
  "Require system-installed test and benchmark dependencies"
  OFF
)

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(GradyXCompilerOptions)
include(GradyXSanitizers)
include(GradyXDependencies)

gradyx_validate_build_options()

add_library(gradyx_core STATIC src/core/version.cc)
add_library(gradyx::core ALIAS gradyx_core)
target_compile_features(gradyx_core PUBLIC cxx_std_20)
target_include_directories(
  gradyx_core
  PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)
set_target_properties(gradyx_core PROPERTIES OUTPUT_NAME gradyx_core)
gradyx_set_project_warnings(gradyx_core)
gradyx_enable_instrumentation(gradyx_core)
gradyx_enable_clang_tidy(gradyx_core)

# Runtime and backend targets establish dependency direction without defining
# their Phase 0.3 interfaces prematurely.
add_library(gradyx_runtime INTERFACE)
add_library(gradyx::runtime ALIAS gradyx_runtime)
target_link_libraries(gradyx_runtime INTERFACE gradyx::core)

add_library(gradyx_backend_cpu INTERFACE)
add_library(gradyx::backend_cpu ALIAS gradyx_backend_cpu)
target_link_libraries(gradyx_backend_cpu INTERFACE gradyx::runtime)

add_library(gradyx INTERFACE)
add_library(gradyx::gradyx ALIAS gradyx)
target_link_libraries(gradyx INTERFACE gradyx::backend_cpu)

if(GRADYX_ENABLE_CUDA)
  message(
    FATAL_ERROR
      "GRADYX_ENABLE_CUDA is reserved for Phase 0.4 and is not implemented yet"
  )
endif()

if(GRADYX_BUILD_TESTS)
  gradyx_setup_googletest()
  add_subdirectory(tests)
endif()

if(GRADYX_BUILD_BENCHMARKS)
  gradyx_setup_benchmark()
  add_subdirectory(benchmarks)
endif()

find_program(GRADYX_CLANG_FORMAT_EXECUTABLE NAMES clang-format)
if(GRADYX_CLANG_FORMAT_EXECUTABLE)
  add_custom_target(
    format
    COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/tools/format.sh"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    COMMENT "Formatting GradyX sources"
  )
  add_custom_target(
    format-check
    COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/tools/format.sh" --check
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    COMMENT "Checking GradyX source formatting"
  )
endif()
