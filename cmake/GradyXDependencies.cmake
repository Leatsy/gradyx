include_guard(GLOBAL)

include(FetchContent)

function(gradyx_setup_googletest)
  find_package(GTest 1.14 CONFIG QUIET)
  if(GTest_FOUND)
    return()
  endif()

  if(GRADYX_USE_SYSTEM_DEPENDENCIES)
    message(
      FATAL_ERROR
        "GoogleTest >= 1.14 was not found and system dependencies are required"
    )
  endif()

  set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
  set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.15.2
    GIT_SHALLOW TRUE
  )
  FetchContent_MakeAvailable(googletest)
endfunction()

function(gradyx_setup_benchmark)
  find_package(benchmark 1.8 CONFIG QUIET)
  if(benchmark_FOUND)
    return()
  endif()

  if(GRADYX_USE_SYSTEM_DEPENDENCIES)
    message(
      FATAL_ERROR
        "Google Benchmark >= 1.8 was not found and system dependencies are required"
    )
  endif()

  set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
  set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
  set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)
  FetchContent_Declare(
    googlebenchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG v1.9.1
    GIT_SHALLOW TRUE
  )
  FetchContent_MakeAvailable(googlebenchmark)
endfunction()
