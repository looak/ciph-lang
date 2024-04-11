cmake_minimum_required(VERSION 3.14)

include(cmake/prelude.cmake)

project(
    ciph-lang
    VERSION 0.1.0
    DESCRIPTION "learning project for getting a deeper understanding of programming languages"
    HOMEPAGE_URL "https://example.com/"
    LANGUAGES CXX
)

include(cmake/project-is-top-level.cmake)
include(cmake/variables.cmake)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

# ---- Declare library ----

add_subdirectory("source/shared")
add_subdirectory("source/compiler")
add_subdirectory("source/vm")

# add_library(
#     ciph-lang_lib OBJECT
#     source/lib.cpp
# )

# target_include_directories(
#     ciph-lang_lib ${warning_guard}
#     PUBLIC
#     "\$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/source>"
# )

# target_compile_features(ciph-lang_lib PUBLIC cxx_std_20)

# find_package(fmt REQUIRED)
# target_link_libraries(ciph-lang_lib PRIVATE fmt::fmt)

# ---- Declare executable ----

# add_executable(ciph-lang_exe source/main.cpp)
# add_executable(ciph-lang::exe ALIAS ciph-lang_exe)

# set_property(TARGET ciph-lang_exe PROPERTY OUTPUT_NAME ciph-lang)

# target_compile_features(ciph-lang_exe PRIVATE cxx_std_20)

# target_link_libraries(ciph-lang_exe PRIVATE ciph-lang_lib)

# ---- Install rules ----

if(NOT CMAKE_SKIP_INSTALL_RULES)
  include(cmake/install-rules.cmake)
endif()

# ---- Developer mode ----

if(NOT ciph-lang_DEVELOPER_MODE)
  return()
elseif(NOT PROJECT_IS_TOP_LEVEL)
  message(
      AUTHOR_WARNING
      "Developer mode is intended for developers of ciph-lang"
  )
endif()

include(cmake/dev-mode.cmake)
