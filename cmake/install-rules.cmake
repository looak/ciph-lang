install(
    TARGETS ciph-compiler_exe
    RUNTIME COMPONENT ciph-lang_Runtime
)

if(PROJECT_IS_TOP_LEVEL)
  include(CPack)
endif()
