add_library(shrimpfile STATIC
    src/shrimpfile.cpp
)

target_include_directories(shrimpfile
PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(shrimpfile PUBLIC common)