# ---- ciph-compiler: source -> bytecode ----

add_executable(ciph-compiler compiler/main.cpp)
target_link_libraries(ciph-compiler PRIVATE ciph::compiler fmt::fmt ciph::warnings)

# ---- ciph-vm: compiles and runs a program ----

add_executable(ciph-vm vm/main.cpp)
target_link_libraries(ciph-vm PRIVATE ciph::vm ciph::compiler fmt::fmt ciph::warnings)
