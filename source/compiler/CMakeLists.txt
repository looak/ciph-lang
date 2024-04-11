set(PROJECT_NAME ciph-compiler_exe)

include(source_list.cmake)

add_executable(${PROJECT_NAME} ${COMPILER_ALL_SRC})

target_link_libraries(${PROJECT_NAME}
  PRIVATE  
  ${SHARED_LIB}
)

target_include_directories(${PROJECT_NAME} 
    PUBLIC
        ${COMPILER_INC_DIR}
        ../shared/inc)