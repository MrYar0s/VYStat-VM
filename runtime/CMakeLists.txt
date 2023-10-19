# Define runtime library build

add_library(runtime STATIC)
add_library(shrimp::runtime ALIAS runtime)

add_subdirectory(common)

# Add runtime_runtime include and src
add_subdirectory(runtime)
# Add runtime_shrimp_vm include and src
add_subdirectory(shrimp_vm)

target_link_libraries(runtime
PUBLIC
    shrimp::common
    shrimp::runtime_common
)
