set(PROJECT_NAME ciph-vm_exe)

find_package(fmt CONFIG REQUIRED)

include(source_list.cmake)

add_executable(${PROJECT_NAME} ${VM_ALL_SRC})

target_link_libraries(${PROJECT_NAME}
  PRIVATE  
  ${SHARED_LIB}
  fmt::fmt
)

target_include_directories(${PROJECT_NAME} 
    PUBLIC
        ${VM_INC_DIR}
        ../shared/inc)