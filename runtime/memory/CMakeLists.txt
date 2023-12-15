# Define runtime::memory interface deps

add_library(runtime_memory INTERFACE)
add_library(shrimp::runtime::memory ALIAS runtime_memory)

target_include_directories(runtime_memory
INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(runtime_memory
INTERFACE
    shrimp::common
)
