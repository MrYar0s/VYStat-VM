# Define runtime::runtime interface deps

add_library(runtime_runtime INTERFACE)
add_library(shrimp::runtime::runtime ALIAS runtime_runtime)

target_include_directories(runtime_runtime
INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(runtime_runtime
INTERFACE
    shrimp::common
)
