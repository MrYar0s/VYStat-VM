# Add runtime_runtime include and src

target_sources(runtime
PRIVATE
    src/runtime.cpp
)

target_include_directories(runtime
PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
