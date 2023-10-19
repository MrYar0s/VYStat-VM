# Add runtime_shrimp_vm include and src

target_sources(runtime
PRIVATE
    src/interpreter.cpp
    src/intrinsics.cpp
)

target_include_directories(runtime
PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
