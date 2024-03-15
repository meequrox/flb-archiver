function(set_compile_options target_name)
  if(MSVC)
    target_compile_options(${target_name} PRIVATE /W4 /WX /EHsc)
  else()
    target_compile_options(${target_name} PRIVATE -Wall -Wextra -Werror -pedantic)
  endif()

  set_target_properties(${target_name} PROPERTIES
    C_STANDARD 11
    C_STANDARD_REQUIRED ON
    C_EXTENSIONS ON
  )

  if(CLANG_TIDY_EXE)
    set_target_properties(${target_name} PROPERTIES
      CXX_CLANG_TIDY ${CLANG_TIDY_EXE}
    )
  endif()
endfunction()
