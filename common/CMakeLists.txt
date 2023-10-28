# Define common library build

add_library(common INTERFACE)
add_library(shrimp::common ALIAS common)

set(COMMON_GEN_ROOT ${CMAKE_CURRENT_BINARY_DIR}/include/shrimp/common)

file(MAKE_DIRECTORY ${COMMON_GEN_ROOT})

shrimp_add_instr_dep_gen(${CMAKE_CURRENT_SOURCE_DIR}/gen_instr_opcode.py ${COMMON_GEN_ROOT}/instr_opcode.gen.hpp
    common common_gen
)

target_include_directories(common
INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}/include
)
