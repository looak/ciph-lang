set(SHARED_LIB "shared_lib")
add_library(${SHARED_LIB} OBJECT)

target_compile_features(${SHARED_LIB} PUBLIC cxx_std_20)

include(source_list.cmake)

target_sources(${SHARED_LIB} 
    PRIVATE 
        ${SHARED_SRC} 
    PUBLIC 
        ${SHARED_INC}
)

target_include_directories(${SHARED_LIB} 
    PUBLIC 
        ${SHARED_INC_DIR}
)