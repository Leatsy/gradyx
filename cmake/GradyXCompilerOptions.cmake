include_guard(GLOBAL)

function(gradyx_set_project_warnings target)
  if(MSVC)
    set(warnings /W4 /permissive-)
  elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(
      warnings
      -Wall
      -Wextra
      -Wpedantic
      -Wconversion
      -Wsign-conversion
      -Wshadow
      -Wnon-virtual-dtor
      -Wold-style-cast
      -Wcast-align
      -Woverloaded-virtual
    )
  else()
    message(
      WARNING
        "No project warning set is defined for ${CMAKE_CXX_COMPILER_ID}"
    )
  endif()

  if(GRADYX_WARNINGS_AS_ERRORS)
    if(MSVC)
      list(APPEND warnings /WX)
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
      list(APPEND warnings -Werror)
    endif()
  endif()

  target_compile_options(${target} PRIVATE ${warnings})
endfunction()

function(gradyx_enable_clang_tidy target)
  if(NOT GRADYX_ENABLE_CLANG_TIDY)
    return()
  endif()

  find_program(GRADYX_CLANG_TIDY_EXECUTABLE NAMES clang-tidy REQUIRED)
  set_target_properties(
    ${target}
    PROPERTIES
      CXX_CLANG_TIDY "${GRADYX_CLANG_TIDY_EXECUTABLE};--warnings-as-errors=*"
  )
endfunction()
