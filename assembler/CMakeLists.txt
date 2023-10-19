# Define assembler module build

find_package(FLEX REQUIRED)

flex_target(lexer
    src/lexer.ll
    ${CMAKE_CURRENT_BINARY_DIR}/lexer.gen.cpp
)

set(ASSEMBLER_GEN_ROOT ${CMAKE_CURRENT_BINARY_DIR}/include/shrimp/assembler)

file(MAKE_DIRECTORY ${ASSEMBLER_GEN_ROOT})

add_custom_command(
    OUTPUT ${ASSEMBLER_GEN_ROOT}/instr.gen.hpp
    COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/gen_instr.py ${INSTR} ${ASSEMBLER_GEN_ROOT}/instr.gen.hpp
    DEPENDS ${INSTR} ${CMAKE_CURRENT_SOURCE_DIR}/gen_instr.py
)

add_custom_command(
    OUTPUT ${ASSEMBLER_GEN_ROOT}/parser.gen.hpp
    COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/gen_parser.py ${INSTR} ${ASSEMBLER_GEN_ROOT}/parser.gen.hpp
    DEPENDS ${INSTR} ${CMAKE_CURRENT_SOURCE_DIR}/gen_parser.py
)

add_executable(assembler
    src/assembler.cpp
    ${ASSEMBLER_GEN_ROOT}/instr.gen.hpp
    ${ASSEMBLER_GEN_ROOT}/parser.gen.hpp
    ${FLEX_lexer_OUTPUTS}
)

target_include_directories(assembler
PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(assembler
PRIVATE
    common
)

target_compile_features(assembler PRIVATE cxx_std_20)
