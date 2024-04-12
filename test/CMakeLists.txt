# Parent project does not export its library target, so this CML implicitly
# depends on being added from it, i.e. the testing is done only from the build
# tree and is not feasible from an install location

project(ciph-langTests LANGUAGES CXX)

# ---- Dependencies ----

find_package(GTest CONFIG REQUIRED)
include(GoogleTest)

set(GTest_LIBRARIES 
    GTest::gtest 
    GTest::gtest_main
    GTest::gmock
    GTest::gmock_main)

find_package(fmt CONFIG REQUIRED)

# ---- Build Compiler as a library ----

include(../source/compiler/source_list.cmake)

set(COMPILER_LIB ciph-compiler_lib)
add_library(${COMPILER_LIB} OBJECT)

target_compile_features(${COMPILER_LIB} PUBLIC cxx_std_20)

target_link_libraries(${COMPILER_LIB} PRIVATE fmt::fmt)

target_sources(${COMPILER_LIB} 
    PRIVATE 
        ${COMPILER_SRC} 
    PUBLIC 
        ${COMPILER_INC}
)

target_include_directories(${COMPILER_LIB} 
    PUBLIC
    ${COMPILER_INC_DIR}
    ../source/shared/inc)

# ---- Build VM as a library ----

include(../source/vm/source_list.cmake)

set(VM_LIB ciph-vm_lib)
add_library(${VM_LIB} OBJECT)

target_compile_features(${VM_LIB} PUBLIC cxx_std_20)

target_sources(${VM_LIB} 
    PRIVATE 
        ${VM_SRC} 
    PUBLIC 
        ${VM_INC}
)

target_include_directories(${VM_LIB} 
    PUBLIC
    ${VM_INC_DIR}
    ../source/shared/inc
    )

# ---- Compiler Tests ----

include(source/compiler_tests/source_list.cmake)

add_executable(compiler_tests ${COMPILER_TEST_SRC})

target_link_libraries(
    compiler_tests PRIVATE
    ${COMPILER_LIB}
    ${GTest_LIBRARIES}    
)

target_include_directories(compiler_tests PUBLIC ${COMPILER_INC_DIR})

target_compile_features(compiler_tests PRIVATE cxx_std_20)

gtest_discover_tests(compiler_tests)
add_dependencies(compiler_tests ${GTest_LIBRARIES})

# ---- VM Tests ----

include(source/vm_tests/source_list.cmake)

add_executable(vm_tests ${VM_TEST_SRC})

target_link_libraries(
    vm_tests PRIVATE
    ${VM_LIB}
    ${GTest_LIBRARIES}
)

target_include_directories(vm_tests PUBLIC ${VM_INC_DIR})

target_compile_features(vm_tests PRIVATE cxx_std_20)

gtest_discover_tests(vm_tests)
add_dependencies(compiler_tests ${GTest_LIBRARIES})

# ---- End-of-file commands ----

add_folders(Test)
