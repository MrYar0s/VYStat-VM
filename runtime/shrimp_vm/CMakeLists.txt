# Add runtime_shrimp_vm include and src

target_sources(runtime
PRIVATE
    src/interpreter.cpp
    src/intrinsics.cpp
)

set(SHRIMP_VM_GEN_ROOT ${CMAKE_CURRENT_BINARY_DIR}/include/shrimp/runtime/shrimp_vm)

file(MAKE_DIRECTORY ${SHRIMP_VM_GEN_ROOT})

shrimp_add_instr_dep_gen(${CMAKE_CURRENT_SOURCE_DIR}/gen_instr.py ${SHRIMP_VM_GEN_ROOT}/instr.gen.hpp
    runtime runtime_instr_gen
)

shrimp_add_instr_dep_gen(${CMAKE_CURRENT_SOURCE_DIR}/gen_dispatch_table.py ${SHRIMP_VM_GEN_ROOT}/dispatch_table.gen.hpp
    runtime runtime_dispatch_gen
)

target_include_directories(runtime
PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}/include
)
