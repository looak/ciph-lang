ciph_add_library(vm
  SOURCES
    src/instructions.cpp
    src/processing_unit.cpp
  HEADERS
    include/ciph/vm/execution_context.hpp
    include/ciph/vm/instructions.hpp
    include/ciph/vm/memory.hpp
    include/ciph/vm/processing_unit.hpp
  DEPENDS
    ciph::shared
  PRIVATE_DEPENDS
    fmt::fmt
)

# Debugger views for the VM's types. The linker embeds a .natvis in the
# executable's PDB (/NATVIS), and this library is static, so the flag goes to
# everything that links ciph::vm. CMake only does this for .natvis sources
# with the Visual Studio generators, not Ninja, hence the explicit option.
# LINK_DEPENDS relinks when the file changes.
if(MSVC)
  set(_ciph_vm_natvis "${CMAKE_CURRENT_SOURCE_DIR}/ciph_vm.natvis")
  target_link_options(ciph_vm INTERFACE "/NATVIS:${_ciph_vm_natvis}")
  set_property(TARGET ciph_vm APPEND PROPERTY INTERFACE_LINK_DEPENDS "${_ciph_vm_natvis}")
endif()
