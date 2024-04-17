set(VM_SRC_DIR ${CMAKE_CURRENT_LIST_DIR}/src)
set(VM_INC_DIR ${CMAKE_CURRENT_LIST_DIR}/inc)

set(VM_SRC 
    ${VM_SRC}
    ${VM_SRC_DIR}/instructions.cpp

    ${VM_SRC_DIR}/processing_unit.cpp    
)

set(VM_INC 
    ${VM_INC}
    ${VM_INC_DIR}/execution_context.hpp    
    ${VM_INC_DIR}/instructions.hpp
    ${VM_INC_DIR}/memory.hpp
    ${VM_INC_DIR}/processing_unit.hpp    
)

set(VM_ALL_SRC 
    ${VM_SRC_DIR}/ciph_vm.cpp # entry point

    ${VM_SRC}
    ${VM_INC})