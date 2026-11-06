# Third-party dependencies.
#
# Each dependency is first looked up with find_package() (so a system install,
# vcpkg or Conan toolchain is used when present) and only downloaded and built
# from source when it isn't found.

include(FetchContent)

# ---- fmt ----

set(FMT_INSTALL OFF CACHE BOOL "" FORCE)

FetchContent_Declare(fmt
  URL https://github.com/fmtlib/fmt/archive/refs/tags/11.2.0.tar.gz
  SYSTEM
  EXCLUDE_FROM_ALL
  FIND_PACKAGE_ARGS 11
)
FetchContent_MakeAvailable(fmt)

# ---- GoogleTest ----

if(CIPH_BUILD_TESTS)
  set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
  set(BUILD_GMOCK ON CACHE BOOL "" FORCE)
  # Match the project's MSVC runtime library (no effect on other compilers).
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

  FetchContent_Declare(googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.tar.gz
    SYSTEM
    EXCLUDE_FROM_ALL
    FIND_PACKAGE_ARGS NAMES GTest
  )
  FetchContent_MakeAvailable(googletest)
endif()
