# Add runtime_shrimp_vm include and src

target_sources(runtime
PRIVATE
    src/interpreter.cpp
    src/intrinsics.cpp
)

set(SHRIMP_VM_GEN_ROOT ${CMAKE_CURRENT_BINARY_DIR}/include/shrimp/runtime/shrimp_vm)

file(MAKE_DIRECTORY ${SHRIMP_VM_GEN_ROOT})

add_custom_command(
    OUTPUT ${SHRIMP_VM_GEN_ROOT}/instr.gen.hpp
    COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/gen_instr.py ${INSTR} ${SHRIMP_VM_GEN_ROOT}/instr.gen.hpp
    DEPENDS ${INSTR} ${CMAKE_CURRENT_SOURCE_DIR}/gen_instr.py
)
add_custom_target(runtime_shrimp_vm_gen DEPENDS ${SHRIMP_VM_GEN_ROOT}/instr.gen.hpp)
add_dependencies(runtime runtime_shrimp_vm_gen)

target_include_directories(runtime
PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}/include
)
