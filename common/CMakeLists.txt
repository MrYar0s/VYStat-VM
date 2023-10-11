
add_library(common INTERFACE)

set(COMMON_GEN_ROOT ${CMAKE_CURRENT_BINARY_DIR}/include/shrimp/common)
set(INST ${PROJECT_ROOT}/inst.yaml)

file(MAKE_DIRECTORY ${COMMON_GEN_ROOT})

add_custom_command(
    OUTPUT ${COMMON_GEN_ROOT}/inst_opcode.gen.hpp
    COMMAND python3 gen_inst_opcode.py ${INST} ${COMMON_GEN_ROOT}/inst_opcode.gen.hpp
    DEPENDS ${INST}
)

target_sources(common
INTERFACE
    ${COMMON_GEN_ROOT}/inst_opcode.gen.hpp
)

target_include_directories(common
INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/include
)
