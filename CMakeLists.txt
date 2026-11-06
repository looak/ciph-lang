cmake_minimum_required(VERSION 3.28)

project(
    ciph-lang
    VERSION 0.1.0
    DESCRIPTION "Learning project for getting a deeper understanding of programming languages"
    LANGUAGES CXX
)

if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
  message(FATAL_ERROR
    "In-source builds are not supported. Use a preset (cmake --preset dev) "
    "or pass a separate build directory, e.g. -B .build/dev")
endif()

# ---- Options ----

option(CIPH_BUILD_TESTS "Build the unit tests" ${PROJECT_IS_TOP_LEVEL})
option(CIPH_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

include(CiphProjectSettings) # output layout, build type, warnings target
include(CiphDependencies)    # fmt, GoogleTest

# ---- Libraries ----

add_subdirectory(source/shared)   # ciph::shared
add_subdirectory(source/compiler) # ciph::compiler
add_subdirectory(source/vm)       # ciph::vm

# ---- Executables ----

add_subdirectory(apps)

# ---- Tests ----

if(CIPH_BUILD_TESTS)
  enable_testing()
  add_subdirectory(test)
endif()

# ---- Install ----

include(GNUInstallDirs)
install(TARGETS ciph-compiler ciph-vm RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
