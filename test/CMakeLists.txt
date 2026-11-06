include(GoogleTest)

# ciph_add_test(<name> SOURCES ... DEPENDS ...)
#
# Test executables stay in the build tree (.build/...) rather than dist/,
# since they are not deliverables.
function(ciph_add_test name)
  cmake_parse_arguments(PARSE_ARGV 1 arg "" "" "SOURCES;DEPENDS")

  add_executable(${name} ${arg_SOURCES})
  target_link_libraries(${name} PRIVATE ${arg_DEPENDS} GTest::gtest_main GTest::gmock ciph::warnings)
  set_target_properties(${name} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>"
  )
  gtest_discover_tests(${name} DISCOVERY_MODE PRE_TEST)
endfunction()

ciph_add_test(compiler_tests
  SOURCES
    compiler/tests_code_generator.cpp
    compiler/tests_lexar.cpp
    compiler/tests_parser.cpp
  DEPENDS
    ciph::compiler
    fmt::fmt
)

ciph_add_test(vm_tests
  SOURCES
    vm/tests_instructions.cpp
    vm/tests_processing_unit.cpp
  DEPENDS
    ciph::vm
)
