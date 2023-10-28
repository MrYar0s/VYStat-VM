# Define assembler module build

find_package(FLEX REQUIRED)

flex_target(lexer
    src/lexer.ll
    ${CMAKE_CURRENT_BINARY_DIR}/lexer.gen.cpp
)

set(ASSEMBLER_GEN_ROOT ${CMAKE_CURRENT_BINARY_DIR}/include/shrimp/assembler)

file(MAKE_DIRECTORY ${ASSEMBLER_GEN_ROOT})

add_executable(assembler
    src/assembler.cpp
    ${FLEX_lexer_OUTPUTS}
)

shrimp_add_instr_dep_gen(${CMAKE_CURRENT_SOURCE_DIR}/gen_instr.py ${ASSEMBLER_GEN_ROOT}/instr.gen.hpp
    assembler assembler_instr_gen
)

shrimp_add_instr_dep_gen(${CMAKE_CURRENT_SOURCE_DIR}/gen_parser.py ${ASSEMBLER_GEN_ROOT}/parser.gen.hpp
    assembler assembler_parser_gen
)

target_include_directories(assembler
PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(assembler PRIVATE common)
