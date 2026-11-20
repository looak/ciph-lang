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

if(TARGET ciph::platform)
  # Small program the process tests start as a child. Kept in the build tree
  # next to the tests; its path is handed to them as a compile definition.
  add_executable(process_test_child platform/process_test_child.cpp)
  target_compile_features(process_test_child PRIVATE cxx_std_20)
  target_link_libraries(process_test_child PRIVATE ciph::warnings)
  set_target_properties(process_test_child PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>"
  )

  ciph_add_test(platform_tests
    SOURCES
      platform/tests_process.cpp
      platform/tests_windows_command_line.cpp
    DEPENDS
      ciph::platform
  )
  target_compile_definitions(platform_tests PRIVATE
    CIPH_TEST_CHILD_PATH="$<TARGET_FILE:process_test_child>"
  )
  add_dependencies(platform_tests process_test_child)
endif()
