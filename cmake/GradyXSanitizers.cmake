include_guard(GLOBAL)

function(gradyx_validate_build_options)
  if(GRADYX_ENABLE_TSAN AND (GRADYX_ENABLE_ASAN OR GRADYX_ENABLE_LSAN))
    message(FATAL_ERROR "TSan cannot be combined with ASan or LSan")
  endif()

  if(
    (GRADYX_ENABLE_ASAN OR GRADYX_ENABLE_UBSAN OR GRADYX_ENABLE_TSAN
     OR GRADYX_ENABLE_LSAN OR GRADYX_ENABLE_COVERAGE)
    AND NOT CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang"
  )
    message(
      FATAL_ERROR
        "The configured instrumentation requires GCC or Clang"
    )
  endif()
endfunction()

function(gradyx_enable_instrumentation target)
  set(sanitizers)

  if(GRADYX_ENABLE_ASAN)
    list(APPEND sanitizers address)
  endif()
  if(GRADYX_ENABLE_UBSAN)
    list(APPEND sanitizers undefined)
  endif()
  if(GRADYX_ENABLE_TSAN)
    list(APPEND sanitizers thread)
  endif()
  if(GRADYX_ENABLE_LSAN AND NOT GRADYX_ENABLE_ASAN)
    list(APPEND sanitizers leak)
  endif()

  if(sanitizers)
    list(JOIN sanitizers "," sanitizer_list)
    target_compile_options(
      ${target}
      PRIVATE -fsanitize=${sanitizer_list} -fno-omit-frame-pointer
    )
    target_link_options(
      ${target}
      PRIVATE -fsanitize=${sanitizer_list} -fno-omit-frame-pointer
    )
  endif()

  if(GRADYX_ENABLE_TSAN AND CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_compile_options(${target} PRIVATE -fno-pie)
    target_link_options(${target} PRIVATE -no-pie)
  endif()

  if(GRADYX_ENABLE_COVERAGE)
    target_compile_options(${target} PRIVATE --coverage -O0 -g)
    target_link_options(${target} PRIVATE --coverage)
  endif()
endfunction()
