# ---- ciph-compiler: source -> bytecode ----

add_executable(ciph-compiler compiler/main.cpp)
target_link_libraries(ciph-compiler PRIVATE ciph::compiler fmt::fmt ciph::warnings)

# ---- ciph-vm: compiles and runs a program ----

add_executable(ciph-vm vm/main.cpp)
target_link_libraries(ciph-vm PRIVATE ciph::vm ciph::compiler fmt::fmt ciph::warnings)

# ---- ciph-studio: node graph editor and debugger ----

if(CIPH_BUILD_STUDIO)
  add_executable(ciph-studio
    studio/build_session.cpp
    studio/main.cpp
    studio/source_panel.cpp
  )
  # The studio starts these as separate programs and expects to find them next
  # to itself, so make sure they are built whenever it is.
  add_dependencies(ciph-studio ciph-compiler ciph-vm)
  target_compile_features(ciph-studio PRIVATE cxx_std_20)
  target_link_libraries(ciph-studio PRIVATE
    ciph::platform
    imgui::imgui
    imgui::texteditor
    imnodes::imnodes
    ciph::warnings
  )
endif()
