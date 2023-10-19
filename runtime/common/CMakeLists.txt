# Define runtime_common interface deps

add_library(runtime_common INTERFACE)
add_library(shrimp::runtime_common ALIAS runtime_common)

target_include_directories(runtime_common
INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
