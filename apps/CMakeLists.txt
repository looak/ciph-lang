# ---- ciph-compiler: source -> bytecode ----

add_executable(ciph-compiler compiler/main.cpp)
target_link_libraries(ciph-compiler PRIVATE ciph::compiler fmt::fmt ciph::warnings)

# ---- ciph-vm: compiles and runs a program ----

add_executable(ciph-vm vm/main.cpp)
target_link_libraries(ciph-vm PRIVATE ciph::vm ciph::compiler fmt::fmt ciph::warnings)

# ---- ciph-studio: node graph editor and debugger ----

if(CIPH_BUILD_STUDIO)
  add_executable(ciph-studio
    studio/main.cpp
    studio/source_panel.cpp
  )
  target_compile_features(ciph-studio PRIVATE cxx_std_20)
  target_link_libraries(ciph-studio PRIVATE
    ciph::compiler
    imgui::imgui
    imgui::texteditor
    imnodes::imnodes
    ciph::warnings
  )
endif()
