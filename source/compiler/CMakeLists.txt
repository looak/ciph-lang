set(PROJECT_NAME ciph-compiler_exe)

# ---- dependencies ----

find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

# ---- executable ----

include(source_list.cmake)
add_executable(${PROJECT_NAME} ${COMPILER_ALL_SRC})

target_link_libraries(${PROJECT_NAME}
  PRIVATE  
  fmt::fmt
  ciph-shared_lib
)

target_include_directories(${PROJECT_NAME} 
    PUBLIC
        ${COMPILER_INC_DIR}
        ${SHARED_INC_DIR})