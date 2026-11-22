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
    compiler/tests_semantic_analyzer.cpp
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
  target_compile_features(process_test_child PRIVATE cxx_std_23)
  target_link_libraries(process_test_child PRIVATE ciph::warnings)
  set_target_properties(process_test_child PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>"
  )

  ciph_add_test(platform_tests
    SOURCES
      platform/tests_executable_path.cpp
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

# ---- Command line tests ----
#
# Run the real executables on files, the way ciph-studio does.

set(_cli_dir "${CMAKE_CURRENT_BINARY_DIR}/cli")
file(MAKE_DIRECTORY "${_cli_dir}")

add_test(NAME cli.compiler.compiles_example
  COMMAND ciph-compiler "${PROJECT_SOURCE_DIR}/examples/while.ciph" -o "${_cli_dir}/while.bin"
)
set_tests_properties(cli.compiler.compiles_example PROPERTIES FIXTURES_SETUP cli_while_bytecode)

# The stepper waits for Enter between instructions. Under CTest there is no
# input, so it runs straight through to the result.
add_test(NAME cli.vm.runs_compiled_example
  COMMAND ciph-vm "${_cli_dir}/while.bin"
)
set_tests_properties(cli.vm.runs_compiled_example PROPERTIES
  FIXTURES_REQUIRED cli_while_bytecode
  PASS_REGULAR_EXPRESSION "Program returned: 5"
)

add_test(NAME cli.compiler.reports_compile_error
  COMMAND ciph-compiler "${CMAKE_CURRENT_SOURCE_DIR}/cli/missing_identifier.ciph" -o "${_cli_dir}/bad.bin"
)
set_tests_properties(cli.compiler.reports_compile_error PROPERTIES
  PASS_REGULAR_EXPRESSION "missing_identifier.ciph:1:[0-9]+: error: "
)

add_test(NAME cli.compiler.rejects_missing_arguments COMMAND ciph-compiler)
set_tests_properties(cli.compiler.rejects_missing_arguments PROPERTIES
  PASS_REGULAR_EXPRESSION "usage: ciph-compiler"
)
