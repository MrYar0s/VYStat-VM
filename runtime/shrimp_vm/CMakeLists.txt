# Define runtime::shrimp_vm interface deps

add_library(runtime_shrimp_vm INTERFACE)
add_library(shrimp::runtime::shrimp_vm ALIAS runtime_shrimp_vm)

target_include_directories(runtime_shrimp_vm
INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
