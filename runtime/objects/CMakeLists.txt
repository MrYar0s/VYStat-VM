add_library(runtime_objects INTERFACE)
add_library(shrimp::runtime::objects ALIAS runtime_objects)

target_include_directories(runtime_objects
INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(runtime_objects
INTERFACE
    shrimp::common
)
