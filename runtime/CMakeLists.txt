# Define runtime library build

add_library(runtime STATIC)
add_library(shrimp::runtime ALIAS runtime)

add_subdirectory(common)
add_subdirectory(runtime)
add_subdirectory(memory)
add_subdirectory(objects)

# Add runtime::interpreter include and src
add_subdirectory(interpreter)

# Add runtime::shrimp_vm include and src
add_subdirectory(shrimp_vm)

target_link_libraries(runtime
PUBLIC
    shrimp::common
    shrimp::runtime::common
    shrimp::runtime::memory
    shrimp::runtime::objects
    shrimp::runtime::runtime
    shrimpfile
)
