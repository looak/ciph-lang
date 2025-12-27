set(COMPILER_SRC_DIR ${CMAKE_CURRENT_LIST_DIR}/src)
set(COMPILER_INC_DIR ${CMAKE_CURRENT_LIST_DIR}/inc)

set(COMPILER_SRC 
    ${COMPILER_SRC}
    ${COMPILER_SRC_DIR}/code_generator.cpp
    ${COMPILER_SRC_DIR}/lexar.cpp
    ${COMPILER_SRC_DIR}/parser.cpp
)

set(COMPILER_INC 
    ${COMPILER_INC}
    ${COMPILER_INC_DIR}/ast.hpp
    ${COMPILER_INC_DIR}/code_generator.hpp
    ${COMPILER_INC_DIR}/lexar.hpp
    ${COMPILER_INC_DIR}/lexar_defines.hpp
    ${COMPILER_INC_DIR}/parser.hpp
    ${COMPILER_INC_DIR}/error_reporter.hpp
    ${COMPILER_INC_DIR}/error_defines.hpp
)

set(COMPILER_ALL_SRC 
    ${COMPILER_SRC_DIR}/ciph_compiler.cpp # entry point

    ${COMPILER_SRC}
    ${COMPILER_INC})