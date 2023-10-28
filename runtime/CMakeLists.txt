# Define runtime library build

add_library(runtime STATIC)
add_library(shrimp::runtime ALIAS runtime)

add_subdirectory(common)
add_subdirectory(shrimp_vm)

# Add runtime::runtime include and src
add_subdirectory(runtime)
# Add runtime::interpreter include and src
add_subdirectory(interpreter)

target_link_libraries(runtime
PUBLIC
    shrimp::common
    shrimp::runtime::common
    shrimp::runtime::shrimp_vm
)
