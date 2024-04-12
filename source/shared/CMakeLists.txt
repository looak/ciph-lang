set(SHARED_LIB "shared_lib")

include(source_list.cmake)

add_library(${SHARED_LIB} OBJECT ${SHARED_SRC_ALL})

target_compile_features(${SHARED_LIB} PUBLIC cxx_std_20)
  

target_include_directories(${SHARED_LIB} 
    PUBLIC 
        ${SHARED_INC_DIR}
)