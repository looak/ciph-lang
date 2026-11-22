# Project-wide settings that are not tied to a single target.

# ---- Default build type (single-config generators only) ----

get_property(_ciph_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(NOT _ciph_multi_config AND NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release RelWithDebInfo MinSizeRel)
endif()

# ---- Output layout ----
#
#   .build/<preset>/            configure + intermediate files (CMake binary dir)
#   dist/<Config>               final executables
#
# Only applied when ciph-lang is the top-level project, so it never writes into
# the source tree of a project that consumes it via add_subdirectory/FetchContent.

if(PROJECT_IS_TOP_LEVEL)
  set(CIPH_DIST_DIR "${PROJECT_SOURCE_DIR}/dist" CACHE PATH "Root directory for build outputs")

  # The $<CONFIG> genex stops multi-config generators from appending their own
  # per-config subdirectory, so both generator kinds produce the same layout.
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CIPH_DIST_DIR}/$<CONFIG>")
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# No C++20 modules in this project; skip the per-file dependency scan.
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

# ---- Warnings ----
#
# Linked PRIVATE into every first-party target. Not applied to dependencies.

add_library(ciph_warnings INTERFACE)
add_library(ciph::warnings ALIAS ciph_warnings)

if(MSVC) # cl.exe and clang-cl
  target_compile_options(ciph_warnings INTERFACE /W4 /permissive- /utf-8)
  if(CIPH_WARNINGS_AS_ERRORS)
    target_compile_options(ciph_warnings INTERFACE /WX)
  endif()
else() # GCC, Clang, AppleClang
  target_compile_options(ciph_warnings INTERFACE -Wall -Wextra -Wpedantic -Wshadow)
  if(CIPH_WARNINGS_AS_ERRORS)
    target_compile_options(ciph_warnings INTERFACE -Werror)
  endif()
endif()

# ---- Helper ----
#
# ciph_add_library(<name> SOURCES ... HEADERS ... [DEPENDS ...] [PRIVATE_DEPENDS ...])
#
# Creates static library `ciph_<name>` with alias `ciph::<name>`. Public headers
# live under include/ciph/<name>/ and are consumed as <ciph/<name>/header.hpp>.

function(ciph_add_library name)
  cmake_parse_arguments(PARSE_ARGV 1 arg "" "" "SOURCES;HEADERS;DEPENDS;PRIVATE_DEPENDS")

  set(target ciph_${name})
  add_library(${target} STATIC)
  add_library(ciph::${name} ALIAS ${target})

  target_sources(${target}
    PRIVATE ${arg_SOURCES}
    PUBLIC
      FILE_SET HEADERS
      BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/include"
      FILES ${arg_HEADERS}
  )

  target_compile_features(${target} PUBLIC cxx_std_23)
  set_target_properties(${target} PROPERTIES CXX_EXTENSIONS OFF)

  target_link_libraries(${target}
    PUBLIC ${arg_DEPENDS}
    PRIVATE ciph::warnings ${arg_PRIVATE_DEPENDS}
  )
endfunction()
