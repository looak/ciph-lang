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

# ---- GLFW, Dear ImGui, imnodes, ImGuiColorTextEdit (ciph-studio only) ----

if(CIPH_BUILD_STUDIO)
  find_package(OpenGL REQUIRED)

  set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
  set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
  set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
  set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

  FetchContent_Declare(glfw
    URL https://github.com/glfw/glfw/archive/refs/tags/3.4.tar.gz
    SYSTEM
    EXCLUDE_FROM_ALL
    FIND_PACKAGE_ARGS 3.3 NAMES glfw3
  )

  # Neither Dear ImGui nor imnodes ships a usable CMake build, so only the
  # sources are fetched (SOURCE_SUBDIR points at nothing on purpose) and the
  # targets are defined below. The docking branch of Dear ImGui is used so
  # panels can be docked and rearranged.
  FetchContent_Declare(imgui
    URL https://github.com/ocornut/imgui/archive/refs/tags/v1.93.0-docking.tar.gz
    SOURCE_SUBDIR _none
  )
  # imnodes has no recent tags; pinned to a commit on master.
  FetchContent_Declare(imnodes
    URL https://github.com/Nelarius/imnodes/archive/2d0c280b07b2f62b120034811cc6e41251673961.tar.gz
    SOURCE_SUBDIR _none
  )
  # goossens' rewrite of ImGuiColorTextEdit. Its tags track Dear ImGui
  # releases, so bump this together with imgui above.
  FetchContent_Declare(imguicolortextedit
    URL https://github.com/goossens/ImGuiColorTextEdit/archive/refs/tags/v1.93.0.tar.gz
    SOURCE_SUBDIR _none
  )
  FetchContent_MakeAvailable(glfw imgui imnodes imguicolortextedit)

  add_library(ciph_imgui STATIC EXCLUDE_FROM_ALL
    "${imgui_SOURCE_DIR}/imgui.cpp"
    "${imgui_SOURCE_DIR}/imgui_demo.cpp"
    "${imgui_SOURCE_DIR}/imgui_draw.cpp"
    "${imgui_SOURCE_DIR}/imgui_tables.cpp"
    "${imgui_SOURCE_DIR}/imgui_widgets.cpp"
    "${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp"
    "${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp"
  )
  add_library(imgui::imgui ALIAS ciph_imgui)
  target_include_directories(ciph_imgui SYSTEM PUBLIC
    "${imgui_SOURCE_DIR}"
    "${imgui_SOURCE_DIR}/backends"
  )
  target_compile_features(ciph_imgui PUBLIC cxx_std_17)
  target_link_libraries(ciph_imgui PUBLIC glfw OpenGL::GL)

  add_library(ciph_imnodes STATIC EXCLUDE_FROM_ALL
    "${imnodes_SOURCE_DIR}/imnodes.cpp"
  )
  add_library(imnodes::imnodes ALIAS ciph_imnodes)
  target_include_directories(ciph_imnodes SYSTEM PUBLIC "${imnodes_SOURCE_DIR}")
  target_link_libraries(ciph_imnodes PUBLIC imgui::imgui)

  add_library(ciph_imgui_texteditor STATIC EXCLUDE_FROM_ALL
    "${imguicolortextedit_SOURCE_DIR}/TextEditor.cpp"
  )
  add_library(imgui::texteditor ALIAS ciph_imgui_texteditor)
  target_include_directories(ciph_imgui_texteditor SYSTEM PUBLIC "${imguicolortextedit_SOURCE_DIR}")
  target_link_libraries(ciph_imgui_texteditor PUBLIC imgui::imgui)
endif()
