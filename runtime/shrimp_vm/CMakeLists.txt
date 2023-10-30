# Add runtime::shrimp_vm include and src

target_sources(runtime
PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/shrimp_vm.cpp
)

target_include_directories(runtime
PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
