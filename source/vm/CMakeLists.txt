set(PROJECT_NAME ciph-vm_exe)

# ---- dependencies ----

find_package(fmt CONFIG REQUIRED)

include(../compiler/source_list.cmake)

set(COMPILER_LIB vm_compiler_lib)
add_library(${COMPILER_LIB} OBJECT)

target_compile_features(${COMPILER_LIB} PUBLIC cxx_std_20)


target_sources(${COMPILER_LIB} 
PRIVATE 
${COMPILER_SRC} 
PUBLIC 
${COMPILER_INC}
)

target_link_libraries(${COMPILER_LIB} PRIVATE ciph-shared_lib fmt::fmt)

target_include_directories(${COMPILER_LIB} 
    PUBLIC
    ${COMPILER_INC_DIR}
    ../source/shared/inc)


# ---- executable ----

include(source_list.cmake)

add_executable(${PROJECT_NAME} ${VM_ALL_SRC})

target_link_libraries(${PROJECT_NAME}
  PRIVATE  
  ${COMPILER_LIB}
  ciph-shared_lib
  fmt::fmt
)

target_include_directories(${PROJECT_NAME} 
    PUBLIC
        ${VM_INC_DIR}
        ../shared/inc
        ../compiler/inc)