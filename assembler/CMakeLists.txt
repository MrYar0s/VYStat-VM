
find_package(FLEX REQUIRED)

flex_target(lexer
    src/lexer.ll
    ${CMAKE_CURRENT_BINARY_DIR}/lexer.gen.cpp
)

add_executable(assembler
    src/lexer.cpp
    ${FLEX_lexer_OUTPUTS}
)

target_include_directories(assembler
PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(assembler PRIVATE cxx_std_20)
