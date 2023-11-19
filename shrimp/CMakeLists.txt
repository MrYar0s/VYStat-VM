# Defines shrimp VM build

add_executable(shrimp src/shrimp.cpp)

target_link_libraries(shrimp
PRIVATE
    shrimp::runtime
    CLI11
)
