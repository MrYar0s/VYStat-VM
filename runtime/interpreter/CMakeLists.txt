# Add runtime::interpreter include and src

target_sources(runtime
PRIVATE
    src/interpreter.cpp
    src/intrinsics.cpp
)

set(SHRIMP_INTERPRETER_GEN_ROOT ${CMAKE_CURRENT_BINARY_DIR}/include/shrimp/runtime/interpreter)

file(MAKE_DIRECTORY ${SHRIMP_INTERPRETER_GEN_ROOT})

shrimp_add_instr_dep_gen(${CMAKE_CURRENT_SOURCE_DIR}/gen_instr.py ${SHRIMP_INTERPRETER_GEN_ROOT}/instr.gen.hpp
    runtime runtime_interpreter_instr_gen
)

shrimp_add_instr_dep_gen(${CMAKE_CURRENT_SOURCE_DIR}/gen_dispatch_table.py ${SHRIMP_INTERPRETER_GEN_ROOT}/dispatch_table.gen.inl
    runtime runtime_interpreter_dispatch_gen
)

target_include_directories(runtime
PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}/include
)
