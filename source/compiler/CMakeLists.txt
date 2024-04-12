set(PROJECT_NAME ciph-compiler_exe)

# ---- dependencies ----

find_package(fmt CONFIG REQUIRED)

include(../shared/source_list.cmake)

set(SHARED_LIB ciph-shared_lib)
add_library(${SHARED_LIB} OBJECT ${SHARED_SRC_ALL})

target_compile_features(${SHARED_LIB} PUBLIC cxx_std_20)
target_include_directories(${SHARED_LIB} PUBLIC ${SHARED_INC_DIR})

# ---- executable ----

include(source_list.cmake)
add_executable(${PROJECT_NAME} ${COMPILER_ALL_SRC})

target_link_libraries(${PROJECT_NAME}
  PRIVATE  
  fmt::fmt
  ${SHARED_LIB}
)

target_include_directories(${PROJECT_NAME} 
    PUBLIC
        ${COMPILER_INC_DIR}
        ${SHARED_INC_DIR})