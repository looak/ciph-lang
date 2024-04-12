set(SHARED_LIB ciph-shared_lib)

# ---- dependencies ----
find_package(fmt CONFIG REQUIRED)


# ---- library ----

include(source_list.cmake)
add_library(${SHARED_LIB} OBJECT ${SHARED_SRC_ALL})

target_link_libraries(${SHARED_LIB} 
PRIVATE 
fmt::fmt
)

target_compile_features(${SHARED_LIB} PUBLIC cxx_std_20)

target_include_directories(${SHARED_LIB} 
    PUBLIC 
        ${SHARED_INC_DIR}        
)